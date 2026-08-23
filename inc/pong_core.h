/*
 * Author: Aidan S
 */

#ifndef __PONG_CORE__
#define __PONG_CORE__

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/drivers/gpio.h>

// include new graphics header
#include "graphics_common.h"

// TODO: rewrite everything

/* Macros */
#define ARENA_TOP SCREEN_LIMIT_Y
#define ARENA_RIG SCREEN_LIMIT_X
#define ARENA_BOT (0U)
#define ARENA_LEF (0U)

/* Arena macros */
#define PADDLE_SIZE (10U) // num of pixels from top to bottom

/* Device macros */
// Devicetree stuff
#define PLAY1_UP_BUT DT_ALIAS(sw0)
#define PLAY1_DN_BUT DT_ALIAS(sw1)
#define PLAY2_UP_BUT DT_ALIAS(sw2)
#define PLAY2_DN_BUT DT_ALIAS(sw3)

extern const struct gpio_dt_spec p1_up_but;
extern const struct gpio_dt_spec p1_dn_but;
extern const struct gpio_dt_spec p2_up_but;
extern const struct gpio_dt_spec p2_dn_but;

extern bool game_in_progress;

// TODO #define BALL_SPEED

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
	BALL_NODIR,
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
