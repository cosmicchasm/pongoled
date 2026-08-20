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
#include <zephyr/kernel.h>

#include "graphics_common.h"
#include "pong_core.h"

/* Local macros */
// Debug
#define PONG_ASSERT(x) __ASSERT((x) == true, "pong assert failed at %d", __LINE__)

/* Static variables */
static uint8_t pong_fb[SCREEN_LIMIT_X * SCREEN_LIMIT_Y / 8];

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

// TODO: Work on filling this stuff out
int pong_main(void) {
  return 0;
}
