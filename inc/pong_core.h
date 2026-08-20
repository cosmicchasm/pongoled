/*
 * Author: Aidan S
 */

#ifndef __PONG_CORE__
#define __PONG_CORE__

#include <stdint.h>

// include new graphics header
#include "graphics_common.h"

// TODO: rewrite everything

/* Macros */
#define ARENA_TOP SCREEN_LIMIT_Y
#define ARENA_RIG SCREEN_LIMIT_X
#define ARENA_BOT (0U)
#define ARENA_LEF (0U)

/* Enums */
typedef enum {
	BALL_W = 0,
	BALL_NW,
	BALL_N,
	BALL_NE,
	BALL_E,
	BALL_SE,
	BALL_S,
	BALL_SW,
	BALL_NUMDIRS
} ball_dir_t;

/* Structures */
typedef struct {
	uint16_t pdl_center; // center of the paddle
	uint16_t points;		 // point count for a player
} player_t;

typedef struct {
	uint16_t xp;
	uint16_t yp;
	ball_dir_t dir;
} ball_t;

/* Function declarations */
void pong_test_page(void);
int pong_main(void);

#endif
