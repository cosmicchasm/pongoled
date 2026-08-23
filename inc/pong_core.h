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

#define BALL_C (0U) // ball change
#define PLA1_U (1U)	// player 1 up
#define PLA2_U (2U)	// player 2 up
#define PLA1_D (3U)	// player 1 down
#define PLA2_D (4U)	// player 2 down
#define P_SCOR (5U) // score event--clear and reset

// old ball location, written by ball thread
#define BALL_X_MSK (0XFF000000)
#define BALL_Y_MSK (0X00FF0000)

/* Arena macros */
// would like this to be odd for equal top/bottom lengths
#define PADDLE_SIZE (11U) // num of pixels from top to bottom

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

/* Extern arena structures */
extern player_t p1;
extern player_t p2;
extern ball_t	 ball;

/* Function declarations */
ball_dir_t update_ball_dir(ball_t *const b);
void pong_test_page(void);
int pong_main(void);

#endif
