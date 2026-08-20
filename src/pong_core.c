/*
 * Author: Aidan S
 */

// Remaking this file since I was very smart and deleted it forever

// stdlib includes here
#include <stdbool.h>
#include <stdint.h>

// Zephyr includes
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>

#include "graphics_common.h"
#include "pong_core.h"

LOG_MODULE_REGISTER(pong_main);

/* Local macros */
// Debug
#define PONG_ASSERT(x) __ASSERT((x) == true, "pong assert failed at %d", __LINE__)

// For test page
#ifdef PONG_TEST_PAGE
static const tp_data[] = {{0,0},{64,32},{127,63},{0,63},{64,32},{127,0}};
#endif

/* Static variables */
static uint8_t pong_fb[SCREEN_LIMIT_X * SCREEN_LIMIT_Y / SCREEN_DIV];

static player_t p1;
static player_t p2;
static ball_t		ball;

static arena_t arena = {
	.pa = p1,
	.pb = p2,
	.b  = ball,
};

/* Helper functions */
// Checker function to ensure a non-faulty collision
static void pong_check_col(bool xl, bool xr, bool yb, bool yt, ball_dir_t dir) {
#ifdef PONG_DEBUG
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
	ball_t ret = b->dir;

	// determine if a collision has happened
	bool xr_col = (b->xp == ARENA_RIG), xl_col = (b->xp == ARENA_LEF);
	bool yt_col = (b->yp == ARENA_TOP), yb_col = (b->yp == ARENA_BOT);

#ifdef PONG_DEBUG
	// call checker
	pong_check_col(xl_col, xr_col, yb_col, yt_col, ret);
#endif

	// return the current direction if no collision
	if (!(xr_col || xl_col || yt_col || yb_col)) {
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
#ifdef PONG_TEST_PAGE
	// 1.
	for (int i = 0; i < ARRAY_SIZE(tp_data); i++) {
		// set the bit
		set_fb_pixel(tp_data[i].x, tp_data[i].y, pong_fb);
		
		// send it
		oled_send_bits(&pong_fb[0], OLED_SCREEN_SIZE, 0);

		// delay slightly
		k_msleep(25);

		// clear it
		clr_fb_pixel(tp_data[i].x, tp_data[i].y, pong_fb);
	}

	memset(pong_fb, 0, sizeof(pong_fb));

	// 2. write horizontal lines
	for (int y = 0; y < SCREEN_LIMIT_Y; y+=SCREEN_DIV) {
		for (int x = 0; x < SCREEN_LIMIT_X; x++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		oled_send_bits(&pong_fb[0], OLED_SCREEN_SIZE, 0);
	}

	for (int x = 0; x < SCREEN_LIMIT_X; x+=SCREEN_DIV) {
		for (int y = 0; y < SCREEN_LIMIT_Y; y++) {
			set_fb_pixel(x, y, pong_fb);
		}
		// send it
		oled_send_bits(&pong_fb[0], OLED_SCREEN_SIZE, 0);
	}

	k_msleep(25);

	memset(pong_fb, 0, sizeof(pong_fb));

	// 4.
	memset(pong_fb, 0XFF, sizeof(pong_fb));
	oled_send_bits(&pong_fb[0], OLED_SCREEN_SIZE, 0);
	k_msleep(2000);

	memset(pong_fb, 0X00, sizeof(pong_fb));
	oled_send_bits(&pong_fb[0], OLED_SCREEN_SIZE, 0);

	return;
#endif
}

// TODO: this should be a thread
int pong_main(void) {
  // initialization stuff here
#ifdef PONG_TEST_PAGE
	test_page();
#endif
	
	while (1) {
		k_msleep(1000);
	}
}
