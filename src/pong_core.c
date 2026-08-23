/*
 * Author: Aidan S
 *
 * Main source file for pong game
 *
 * Thread definitions in pong_threads.c
 */

// Remaking this file since I was very smart and deleted it forever

// stdlib includes here
#include <stdbool.h>
#include <stdint.h>

// Zephyr includes
#include <zephyr/sys/__assert.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>

#include "graphics_common.h"
#include "pong_threads.h"
#include "pong_core.h"

LOG_MODULE_REGISTER(pong_main);

/* Thread definitions here (is this the right place?) */
K_THREAD_DEFINE(screen_thread_tid, SCREEN_STACK_SIZE, screen_thread,
    NULL, NULL, NULL, SCREEN_PRIORITY, 0, 0);

K_THREAD_DEFINE(ball_thread_tid, BALL_STACK_SIZE, ball_thread,
    NULL, NULL, NULL, BALL_PRIORITY, 0, 0);

K_THREAD_DEFINE(main_thread_tid, MAIN_STACK_SIZE, pong_main,
		NULL, NULL, NULL, MAIN_PRIORITY, 0, 0);

// assert helper -- assumes CONFIG_ASSERT=y for now
#define PONG_ASSERT_COLLISION(X) __ASSERT((X) == true, "improper collision")

const struct gpio_dt_spec p1_up_but = GPIO_DT_SPEC_GET(PLAY1_UP_BUT, gpios);
const struct gpio_dt_spec p2_up_but = GPIO_DT_SPEC_GET(PLAY2_UP_BUT, gpios);
const struct gpio_dt_spec p1_dn_but = GPIO_DT_SPEC_GET(PLAY1_DN_BUT, gpios);
const struct gpio_dt_spec p2_dn_but = GPIO_DT_SPEC_GET(PLAY2_DN_BUT, gpios);

/* Local macros */
// Debug
#define PONG_ASSERT(x) __ASSERT((x) == true, "pong assert failed at %d", __LINE__)

// For test page
static const pos_t tp_data[] = {{0,0},{64,32},{127,63},{0,63},{64,32},{127,0}};

/* Variables */
static PIX_TYPE pong_fb[SCREEN_LIMIT_X * SCREEN_LIMIT_Y / SCREEN_DIV];

static player_t p1;
static player_t p2;
static ball_t		ball;

/* Helper functions */
// Checker function to ensure a non-faulty collision
static void pong_check_col(bool xl, bool xr, bool yb, bool yt, ball_dir_t dir) {
#if (defined(PONG_DEBUG))
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

// Replica of the gross function from earlier
static ball_dir_t update_ball_dir(ball_t *const b) {
	ball_dir_t ret = b->dir;

	// determine if a collision with a wall has happened
	const bool xr_col = (b->xp == ARENA_RIG), xl_col = (b->xp == ARENA_LEF);
	const bool yt_col = (b->yp == ARENA_TOP), yb_col = (b->yp == ARENA_BOT);

	// call checker
	pong_check_col(xl_col, xr_col, yb_col, yt_col, ret);

	// TODO: detect a collision with a paddle here
	bool pad_col = false;

	if (!(xr_col || xl_col || yt_col || yb_col)) {
		// return the current direction if no collision
		return ret;
	} else if ((xr_col || xl_col) && !pad_col) {
		// return an invalid direction if collision with RL walls
		// since the round is over
		ret = BALL_NODIR;

		// TODO: signal to main thread to reset game
		return ret;
	}

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
			ret = (yt_col) ? BALL_SW : (xl_col) ? BALL_NE : ret;
			break;
		case BALL_NE:
			ret = (yt_col) ? BALL_SE : (xr_col) ? BALL_NW : ret;
			break;
		case BALL_SW:
			ret = (yb_col) ? BALL_NW : (xl_col) ? BALL_SE : ret;
			break;
		case BALL_SE:
			ret = (yb_col) ? BALL_NE : (xr_col) ? BALL_SW : ret;
			break;
		default:
			break;
	}
	return ret;
}

// test page implementation here
// maybe we should have this as a lifecycle init type function?

/*
 * 1. Test writing a set of pixels to screen, then clearing
 * 2. Test writing a set of horizontal/vertical lines to screen, then clearing
 * 3. Test writing diagonal lines, then clearing
 * 4. Test writing whole screen, then clearing
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
	for (int y = 0; y < SCREEN_LIMIT_Y; y+=SCREEN_DIV) {
		for (int x = 0; x < SCREEN_LIMIT_X; x++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		write_display(&pong_fb[0], OLED_SIZE);
	}

	for (int x = 0; x < SCREEN_LIMIT_X; x+=SCREEN_DIV) {
		for (int y = 0; y < SCREEN_LIMIT_Y; y++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		write_display(&pong_fb[0], OLED_SIZE);
	}

	k_msleep(25);

	memset(pong_fb, 0, sizeof(pong_fb));

	// 4.
  LOG_INF("sending test 4");
	memset(pong_fb, 0XFF, sizeof(pong_fb));
	write_display(&pong_fb[0], OLED_SIZE);
	k_msleep(2000);

	memset(pong_fb, 0X00, sizeof(pong_fb));
	write_display(&pong_fb[0], OLED_SIZE);

  LOG_INF("test page complete");
	return;
}

// TODO: this should be a thread probably
int pong_main(void) {
  // initialization stuff here
  LOG_INF("entered pong_main");
	pong_test_page();
	
  LOG_INF("came back from test page");

	// initialize gpios here
	if (!gpio_is_ready_dt(&p1_up_but)) {
		LOG_ERR("GPIO peripheral not ready");
	}

	/* ... for other GPIOs if necessary */
	gpio_flags_t gpio_cfg = GPIO_INPUT | GPIO_PULL_DOWN;
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

  // once we're done, we start those threads!
  k_thread_start(screen_thread_tid);
  // k_thread_start(player_thread_tid);

  // probably wait here in case we decide to come back for whatever reason
  while (1) { }
}
