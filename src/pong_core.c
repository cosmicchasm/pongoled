/*
 * Author: Aidan S
 *
 * Helper functions for pong game
 * Maybe a bit of a misnomer...?
 *
 * Thread definitions (including main) in pong_threads.c
 */

// stdlib includes here
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// Zephyr includes
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>

#include "graphics_common.h"
#include "pong_core.h"

LOG_MODULE_REGISTER(pong_main);

const struct gpio_dt_spec p1_up_but = GPIO_DT_SPEC_GET(PLAY1_UP_BUT, gpios);
const struct gpio_dt_spec p2_up_but = GPIO_DT_SPEC_GET(PLAY2_UP_BUT, gpios);
const struct gpio_dt_spec p1_dn_but = GPIO_DT_SPEC_GET(PLAY1_DN_BUT, gpios);
const struct gpio_dt_spec p2_dn_but = GPIO_DT_SPEC_GET(PLAY2_DN_BUT, gpios);

/* Local macros */
// Debug
#if (defined(ASSERT) && ASSERT)
#define PONG_ASSERT(x) __ASSERT((x) == true, "pong assert failed at %d", __LINE__)
#endif

// For test page
static const pos_t tp_data[] = {{0,0},{64,32},{127,63},{0,63},{64,32},{127,0}};

/* Variables */
static PIX_TYPE pong_fb[SCREEN_SIZE];
bool game_in_progress = false;

static player_t p1;
static player_t p2;
static ball_t	 ball;

struct k_thread player_thread_obj;
struct k_thread screen_thread_obj;
struct k_thread ball_thread_obj;

K_THREAD_STACK_DEFINE(player_stack, PLAYER_STACK_SIZE);
K_THREAD_STACK_DEFINE(screen_stack, SCREEN_STACK_SIZE);
K_THREAD_STACK_DEFINE(ball_stack, BALL_STACK_SIZE);

/* Message queue definition */
// This is what the control threads will use to communicate with
// the screen
static char msgq_buf[PONG_NUM_MSGS * sizeof(uint32_t)];
struct k_msgq pong_msgq;

/* Semaphore definition */
struct k_sem pong_sem;

/* Event definition */
struct k_event mode_event;

/* Helper functions */
// Checker function to ensure a non-faulty collision
static void pong_check_col(bool xl, bool xr, bool yb, bool yt, ball_dir_t dir) {
#if (defined(PONG_DEBUG) && defined(ASSERT))
	// big switch case
	switch (dir) {
		case BALL_N:
			PONG_ASSERT(yt && (!xl) && (!xr) && (!yb));
		case BALL_W:
			PONG_ASSERT(xl && (!xr) && (!yb) && (!yt));
		case BALL_E:
			PONG_ASSERT(xr && (!xl) && (!yb) && (!yt));
		case BALL_S:
			PONG_ASSERT(yb && (!xr) && (!xl) && (!yt));
		case BALL_NW:
			PONG_ASSERT((yt || xl) && (!xr) && (!yb));
		case BALL_NE:
			PONG_ASSERT((yt || xr) && (!xl) && (!yb));
		case BALL_SW:
			PONG_ASSERT((yb || xl) && (!xr) && (!yt));
		case BALL_SE:
			PONG_ASSERT((yb || xr) && (!xl) && (!yt));
		default:
			if (!(xl || xr || yb || yt)) {
				// no collision
				return;
			} else {
				// collision case that wasn't handled properly
				PONG_ASSERT(false);
			}
	}
#else
	// all arguments unused
	(void)xl;
	(void)xr;
	(void)yb;
	(void)yt;
	(void)dir;
#endif
}

/* helper function for paddle detection */
static inline bool paddle_col_helper() {
  const bool yp_p1 = (ball.yp<=p1.pdl_center+(PADDLE_SIZE>>1)) &&
                     (ball.yp>=p1.pdl_center-(PADDLE_SIZE>>1));
  const bool yp_p2 = (ball.yp<=p2.pdl_center+(PADDLE_SIZE>>1)) &&
                     (ball.yp>=p2.pdl_center-(PADDLE_SIZE>>1));

  return ((ball.xp==ARENA_LEF+1) && yp_p1) ||
         ((ball.xp==ARENA_RIG-2) && yp_p2);
}

ball_dir_t update_ball_dir(ball_t *const b) {
	ball_dir_t ret = b->dir;

	// determine if a collision with a wall has happened
	const bool xr_col = (b->xp == ARENA_RIG), xl_col = (b->xp == ARENA_LEF);
	const bool yt_col = (b->yp == ARENA_TOP), yb_col = (b->yp == ARENA_BOT);

	// call checker
	pong_check_col(xl_col, xr_col, yb_col, yt_col, ret);

	// TODO: use semaphore on player here
	const bool pad_col = paddle_col_helper();

	if (!(xr_col || xl_col || yt_col || yb_col || pad_col)) {
		// return the current direction if no collision
		return ret;
	} else if ((xr_col || xl_col) && !pad_col) {
		// return an invalid direction if collision with RL walls
		// since the round is now over
		ret = BALL_NODIR;

		return ret;
	}

	LOG_INF("ball dir: %d, ball x = %d, ball y = %d", b->dir, b->xp, b->yp);

	// determine new direction manually
	switch (ret) {
		case BALL_W:
			ret = BALL_E;
			break;
		case BALL_E:
			ret = BALL_W;
			break;
		case BALL_N:
			ret = BALL_S;
			break;
		case BALL_S:
			ret = BALL_N;
			break;
		case BALL_NW:
			ret = (yt_col) ? BALL_SW : (pad_col) ? BALL_NE : ret;
			break;
		case BALL_NE:
			ret = (yt_col) ? BALL_SE : (pad_col) ? BALL_NW : ret;
			break;
		case BALL_SW:
			ret = (yb_col) ? BALL_NW : (pad_col) ? BALL_SE : ret;
			break;
		case BALL_SE:
			ret = (yb_col) ? BALL_NE : (pad_col) ? BALL_SW : ret;
			break;
		default:
			break;
	}
	LOG_INF("ball new dir: %d", ret);

  b->dir = ret;

	return ret;
}

// test page implementation here
// maybe we should have this as a lifecycle init type function?

/*
 * 1. Test writing a set of pixels to screen, then clearing
 * 2. Test writing a set of horizontal/vertical lines to screen, then clearing
 * 3. Test writing whole screen, then clearing
 */
void pong_test_page(void) {
	// 1.
  LOG_INF("sending test 1");
	for (int i = 0; i < ARRAY_SIZE(tp_data); i++) {
		// set the bit
		set_fb_pixel(tp_data[i].x, tp_data[i].y, pong_fb);
		
		// send it
		write_display(&pong_fb[0], OLED_SIZE);

		// delay slightly
		k_msleep(25);

		// clear it
		clr_fb_pixel(tp_data[i].x, tp_data[i].y, pong_fb);
	}

	memset(pong_fb, 0, sizeof(pong_fb));

	// 2. write horizontal lines
  LOG_INF("sending test 2");
	for (int y = ARENA_BOT; y < ARENA_TOP; y+=SCREEN_DIV) {
		for (int x = ARENA_LEF; x < ARENA_RIG; x++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		write_display(&pong_fb[0], OLED_SIZE);
	}

	for (int x = ARENA_LEF; x < ARENA_RIG; x+=SCREEN_DIV) {
		for (int y = ARENA_BOT; y < ARENA_TOP; y++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		write_display(&pong_fb[0], OLED_SIZE);
	}

	k_msleep(25);

	memset(pong_fb, 0, sizeof(pong_fb));

	// 3.
  LOG_INF("sending test 4");
	memset(pong_fb, 0XFF, sizeof(pong_fb));
	write_display(&pong_fb[0], OLED_SIZE);
	k_msleep(2000);

	memset(pong_fb, 0X00, sizeof(pong_fb));
	write_display(&pong_fb[0], OLED_SIZE);

  LOG_INF("test page complete");
	return;
}

/* Thread definitions here */

// Thread 1: screen thread
// We can sleep this thread until an arena update of some sort occurs
// We could put this on a work queue instead, although I'd want it
// serviced a little earlier maybe

/*
 * ___Highest p__        _____________       ___Lowest p__
 * |            |        |           |       |           |
 * |   Screen   | ---->  |  Players  | ----> |    Ball   |
 * |____________|        |___________|       |___________|
 */

/* Static procedures */
// true: clear p1
// false: clear p2
static void clr_pong_pdl(bool p, PIX_TYPE *fb) {
	for (int i = ARENA_BOT; i < ARENA_TOP; i++) {
		if (p) {
			clr_fb_pixel(ARENA_LEF, i, fb);
		} else {
			clr_fb_pixel(ARENA_RIG-1, i, fb);
		}
	}
}

void screen_thread(void *a) {

	// passed parameters
	PIX_TYPE *fb = (PIX_TYPE *)a;

	// local variables
	uint32_t rcv_msg;
  bool do_write = false;

  uint8_t ball_x = ball.xp, ball_y = ball.yp;

	// forever loop
	while (1) {
		// if we've paused the game for whatever reason, wait
    // other threads use semaphores but this doesn't... this ok?

    int ret = k_sem_take(&pong_sem, K_FOREVER);

		if (game_in_progress && (0 == ret)) {

			// wait until message received
			while (0 != k_msgq_get(&pong_msgq, &rcv_msg, K_FOREVER)) { }

      do_write = true;

			/* process msgs if any */
			// player handles
			if ((rcv_msg & BIT(PLA1_U)) || (rcv_msg & BIT(PLA1_D))) {
				clr_pong_pdl(true, fb);

				for (int i = -(PADDLE_SIZE>>1); i < (PADDLE_SIZE>>1); i++) {
					set_fb_pixel(0, p1.pdl_center - i, fb);
				}
			}

			if ((rcv_msg & BIT(PLA2_U)) || (rcv_msg & BIT(PLA2_D))) {
				clr_pong_pdl(false, fb);

				for (int i = -(PADDLE_SIZE>>1); i < (PADDLE_SIZE>>1); i++) {
					set_fb_pixel(SCREEN_LIMIT_X-1, p2.pdl_center - i, fb);
				}
			}

			// ball handles
			if (rcv_msg & BIT(BALL_C)) {
				// clear the old ball position from fb
				clr_fb_pixel(ball_x, ball_y, fb);

				// FIELD_GET -> extracts bitfield from rcv_msg and
				// shifts down to LSB
				ball_x = FIELD_GET(BALL_X_MSK, rcv_msg);
				ball_y = FIELD_GET(BALL_Y_MSK, rcv_msg);

				// modify ball position here and write to screen
				set_fb_pixel(ball_x, ball_y, fb);
			}
		}

		/* write the updated frame buffer to the display */
    if (do_write) {
      if (0 != write_display(fb, SCREEN_SIZE)) {
        // do something here
      }
      do_write = false;
    }

    // send event now
    if (rcv_msg & BIT(P_SCOR)) {
			// LOG_INF("sending message to main thread");
      k_event_set(&mode_event, 0X1);

      ball_x = (ARENA_RIG - ARENA_LEF) >> 1;
      ball_y = (ARENA_TOP - ARENA_BOT) >> 1;

      k_yield();
    }
		// LOG_INF("screen wrote message 0X%X", rcv_msg);

    k_sem_give(&pong_sem);
	}
}

// this could lowkey (i guess) submit work to a workqueue or something
// with an interrupt... that'd probably be fine
// params: a -> pointer to player 1 struct
// 				 b -> pointer to player 2 struct
void player_thread(void) {

	uint16_t *cen_p1 = &p1.pdl_center;
	uint16_t *cen_p2 = &p2.pdl_center;

	// forever loop
	while (1) {
		uint32_t p_msg = 0;

		// if we've paused the game for whatever reason, wait
		if (game_in_progress) {
			// there are a couple of ways to do this (like using an interrupt)
			// but i'd (for now) like to keep this to a thread

			// handle--we give precedence to the up button
			if (1 == gpio_pin_get_dt(&p1_up_but)) {
				uint16_t t = *cen_p1;
				if (t < (SCREEN_LIMIT_Y-(PADDLE_SIZE>>1))) {
					// modify struct
					*cen_p1 = t+1;

					// pack msg
					p_msg |= BIT(PLA1_U);
				}
			} else if (1 == gpio_pin_get_dt(&p1_dn_but)) {
				uint16_t t = *cen_p1;

				if (t > (PADDLE_SIZE>>1)) {
					*cen_p1 = t-1;
					p_msg |= BIT(PLA1_D);
				}
			}

			if (1 == gpio_pin_get_dt(&p2_up_but)) {
				uint16_t t = *cen_p2;
				if (t < (SCREEN_LIMIT_Y-(PADDLE_SIZE>>1))) {
					*cen_p2 = t+1;
					p_msg |= BIT(PLA2_U);
				}
			} else if (1 == gpio_pin_get_dt(&p2_dn_but)) {
				uint16_t t = *cen_p2;
				if (t > (PADDLE_SIZE>>1)) {
					*cen_p2 = t-1;
					p_msg |= BIT(PLA2_D);
				}
			}

			if (0 != p_msg) {
				k_msgq_put(&pong_msgq, (const void *)&p_msg, K_NO_WAIT);
				}
			}
			k_msleep(PLAYER_SLEEP_MS);
	}
}

// same with this maybe, but i kinda would want a thread for it if we
// want a pretty specific speed
void ball_thread(void) {
	uint32_t b_msg;

	while (1) {
		if (game_in_progress) {
			b_msg = 0;

			ball_dir_t new_dir = update_ball_dir(&ball);

			// modify ball position
      // TODO: could be cleaner
			switch (new_dir) {
				case BALL_W:
					ball.xp -= 1;
					break;
				case BALL_NW:
					ball.xp -= 1;
					ball.yp += 1;
					break;
				case BALL_N:
					ball.yp += 1;
					break;
				case BALL_NE:
					ball.xp += 1;
					ball.yp += 1;
					break;
				case BALL_E:
					ball.xp += 1;
					break;
				case BALL_SE:
					ball.xp += 1;
					ball.yp -= 1;
					break;
				case BALL_S:
					ball.yp -= 1;
					break;
				case BALL_SW:
					ball.xp -= 1;
					ball.yp -= 1;
					break;
				case BALL_NODIR:
					// will send signal to reset game
					b_msg |= BIT(P_SCOR);
					break;
				default:
					break;
			}
			
			// insert new position here
			if (!(b_msg & BIT(P_SCOR))) {
				b_msg |= FIELD_PREP(BALL_X_MSK, ball.xp);
				b_msg |= FIELD_PREP(BALL_Y_MSK, ball.yp);
				b_msg |= BIT(BALL_C);
			}

			int ret = k_msgq_put(&pong_msgq, (const void *)&b_msg, K_NO_WAIT);

			// then sleep
			k_msleep(BALL_SLEEP_MS);
		} else {
			// k_msleep(PAUSE_SLEEP_MS);
		}
		k_yield();
	}
}

static void pong_initialize_arena(void) {
	LOG_INF("running pong_initialize_arena");

  // initialize the variables
  p1.pdl_center = (ARENA_TOP - ARENA_BOT) >> 1;
  p2.pdl_center = (ARENA_TOP - ARENA_BOT) >> 1;
  ball.xp = (ARENA_RIG - ARENA_LEF) >> 1;
  ball.yp = (ARENA_TOP - ARENA_BOT) >> 1;
  ball.dir = BALL_NE;

  // write to the frame buffer
  memset(pong_fb, 0, SCREEN_SIZE);
  for (int i = -(PADDLE_SIZE>>1); i < (PADDLE_SIZE>>1); i++) {
    // p1
    set_fb_pixel(ARENA_LEF, (int)p1.pdl_center + i, pong_fb);
    set_fb_pixel(ARENA_RIG-1, (int)p2.pdl_center + i, pong_fb);
  }

  // set ball
  set_fb_pixel(ball.xp, ball.yp, pong_fb);

  // write to screen
  write_display(pong_fb, SCREEN_SIZE);

  // sleep
  k_msleep(1000);

  return;
}

void pong_main(void) {
  game_mode_t pong_mode = WAITING;

  // initialization stuff here
  LOG_INF("entered pong_main");

	pong_test_page();
	
  LOG_INF("came back from test page");

	// initialize gpios here
	if (!gpio_is_ready_dt(&p1_up_but)) {
		LOG_ERR("GPIO peripheral not ready");
	}

	/* ... for other GPIOs if necessary */
	gpio_flags_t gpio_cfg = (GPIO_INPUT);
	if (0 != gpio_pin_configure_dt(&p1_up_but, gpio_cfg)) {
		LOG_ERR("p1_up_but cfg fail");
	}
	if (0 != gpio_pin_configure_dt(&p2_up_but, gpio_cfg)) {
		LOG_ERR("p2_up_but cfg fail");
	}
	if (0 != gpio_pin_configure_dt(&p1_dn_but, gpio_cfg)) {
		LOG_ERR("p1_dn_but cfg fail");
	}
	if (0 != gpio_pin_configure_dt(&p2_dn_but, gpio_cfg)) {
		LOG_ERR("p2_dn_but cfg fail");
	}

	/* any other initializations here */

	/* start game screen */
  pong_initialize_arena();

  /* initialize the semaphores here */
  k_sem_init(&pong_sem, 1, 1);
  
	/* initialize the message queue here */
	k_msgq_init(&pong_msgq, msgq_buf, sizeof(uint32_t), PONG_NUM_MSGS);

  /* initialize event variable here */
  k_event_init(&mode_event);

  // threads start immediately on creation
	game_in_progress = true;

  k_tid_t player_tid = k_thread_create(&player_thread_obj, player_stack, K_THREAD_STACK_SIZEOF(player_stack),
                                       player_thread, NULL, NULL, NULL, PLAYER_PRIORITY,
                                       0, K_FOREVER);
  LOG_INF("player_tid: %p", player_tid);

  k_tid_t screen_tid = k_thread_create(&screen_thread_obj, screen_stack, K_THREAD_STACK_SIZEOF(screen_stack),
                                       screen_thread, (void *)&pong_fb, NULL, NULL,
                                       SCREEN_PRIORITY, 0, K_FOREVER);

  LOG_INF("screen_tid: %p", screen_tid);

  k_tid_t ball_tid = k_thread_create(&ball_thread_obj, ball_stack, K_THREAD_STACK_SIZEOF(ball_stack),
                                     ball_thread, NULL, NULL, NULL, BALL_PRIORITY,
                                     0, K_FOREVER);
  LOG_INF("ball_tid: %p", ball_tid);

  pong_mode = RUNNING;

  LOG_INF("game started");

	k_thread_start(ball_tid);
	k_thread_start(player_tid);
	k_thread_start(screen_tid);

  // probably wait here in case we decide to come back for whatever reason
  while (1) {
    LOG_INF("main loop");
    switch (pong_mode) {
      case RUNNING:
        // wait for the event to switch modes
        // remove the event when done
        k_event_wait_safe(&mode_event, 0X1, true, K_FOREVER);
				LOG_INF("message received in main thread");

        // grab the semaphore immediately and block the other threads
        k_sem_take(&pong_sem, K_FOREVER);
        game_in_progress = false;

				LOG_INF("main has semaphore");

        pong_mode = SCORING;
        break;
      case SCORING:
				game_in_progress = false;

        // reintialize screen
        pong_initialize_arena();

        // purge the queue
        k_msgq_purge(&pong_msgq);

        pong_mode = WAITING;
        break;
      case WAITING:
        k_sem_give(&pong_sem);
        pong_mode = RUNNING;
				game_in_progress = true;
        break;
      default:
        k_yield();
        break;
    }
  }
}
