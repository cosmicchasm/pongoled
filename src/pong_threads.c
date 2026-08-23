/*
 * Author: Aidan Stanford
 *
 * This file contains all the thread definitions for the pong game
 * including the main thread
 */

/* Stdlib includes */
#include <stdbool.h>

/* Zephyr includes */
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* Project includes */
#include "graphics_common.h"
#include "pong_threads.h"
#include "pong_core.h"

/* Message queue definition */
// This is what the control threads will use to communicate with
// the screen
static char msgq_buf[PONG_NUM_MSGS * sizeof(uint32_t)];
static struct k_msgq pong_msgq;

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

/* Static variables */

/* Static procedures */
static void clr_pong_pdl(bool p1, bool p2, PIX_TYPE *fb) {
	for (int i = 0; i < SCREEN_LIMIT_Y; i++) {
		if (p1) {
			clr_fb_pixel(0, i, fb);
		}

		if (p2) {
			clr_fb_pixel(SCREEN_LIMIT_X-1, i, fb);
		}
	}
}

void screen_thread(void *a, void *b) {

	// passed parameters
	PIX_TYPE *fb = (PIX_TYPE *)a;
	int sz = *(int *)b;

	// local variables
	uint32_t rcv_msg;
	int8_t ball_x, ball_y;

	// initialize the message queue here
	k_msgq_init(&pong_msgq, msgq_buf, sizeof(uint32_t), PONG_NUM_MSGS);

	// forever loop
	while (1) {
		// if we've paused the game for whatever reason, wait
		if (game_in_progress &&
				(0 == k_msgq_get(&pong_msgq, (uint32_t *)&rcv_msg, K_NO_WAIT))) {

			/* process msgs if any */

			// player handles
			if ((rcv_msg & BIT(PLA1_U)) || (rcv_msg & BIT(PLA1_D))) {
				clr_pong_pdl(true, false, fb);

				for (int i = -(PADDLE_SIZE>>1); i < (PADDLE_SIZE>>1); i++) {
					set_fb_pixel(0, p1.pdl_center - i, fb);
				}
			}

			if ((rcv_msg & BIT(PLA2_U)) || (rcv_msg & BIT(PLA2_D))) {
				clr_pong_pdl(false, true, fb);

				for (int i = -(PADDLE_SIZE>>1); i < (PADDLE_SIZE>>1); i++) {
					set_fb_pixel(SCREEN_LIMIT_X-1, p1.pdl_center - i, fb);
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

			// clear handle
			if (rcv_msg & BIT(P_SCOR)) {
				memset(fb, 0, sz);
			}
		} else if (game_in_progress) {
			/* write the updated frame buffer to the display */
			if (0 != write_display(fb, sz)) {
				LOG_WRN("write display warning, line %d", __LINE__);
			}
			k_yield();
		} else {
			k_msleep(PAUSE_SLEEP_MS);
		}
	}
}

// this could lowkey (i guess) submit work to a workqueue or something
// with an interrupt... that'd probably be fine
// params: a -> pointer to player 1 struct
// 				 b -> pointer to player 2 struct
void player_thread(void) {

	uint16_t *cen_p1 = &p1->pdl_center;
	uint16_t *cen_p2 = &p2->pdl_center;

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

			// send msgs here, if any
			if (0 != p_msg) {
				while (0 != k_msgq_put(&pong_msgq, (const void *)&p_msg, K_NO_WAIT)) { }
			}

			// sleep afterwards
			k_msleep(PLAYER_SLEEP_MS);
		} else {
			k_msleep(PAUSE_SLEEP_MS);
		}
	}
}

// same with this maybe, but i kinda would want a thread for it if we
// want a pretty specific speed
void ball_thread(void) {
	uint32_t b_msg;

	while (1) {
		// if we've paused the game for whatever reason, wait

		if (game_in_progress) {
			// clear message
			b_msg = 0;

			// call update_dir here
			ball_dir_t new_dir = update_ball_dir(&ball);

			// modify ball position
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
					// send signal to reset game here
					b_msg |= P_SCOR;
					break;
				default:
					break;
			}
			
			// insert new position here
			if (!(b_msg & BIT(P_SCOR))) {
				b_msg |= FIELD_PREP(BALL_X_MSK, ball.xp);
				b_msg |= FIELD_PREP(BALL_Y_MSK, ball.yp);
				b_msg |= BALL_C;
			}

			// send message here
			while (0 != k_msgq_put(&pong_msgq, (const void *)&p_msg, K_NO_WAIT)) { }

			// then sleep
			k_msleep(BALL_SLEEP_MS);
		} else {
			k_msleep(PAUSE_SLEEP_MS);
		}
	}
}
