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
#define PADDLE_SIZE (19) // num of pixels from top to bottom

/* Device macros */
// Devicetree stuff
#define PLAY1_UP_BUT DT_ALIAS(sw0)
#define PLAY1_DN_BUT DT_ALIAS(sw1)
#define PLAY2_UP_BUT DT_ALIAS(sw2)
#define PLAY2_DN_BUT DT_ALIAS(sw3)

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
ball_dir_t update_ball_dir(ball_t *const b);
void pong_test_page(void);

/* Thread stuff here */

/* Declarations */
void screen_thread(void *a, void *b, void *c);
void player_thread(void *a, void *b, void *c);
void ball_thread(void *a, void *b, void *c);
void pong_main(void *a, void *b, void *c);

/* Priorities */
#define SCREEN_PRIORITY    10
#define PLAYER_PRIORITY    12
#define BALL_PRIORITY      11
#define PONG_MAIN_PRIORITY 8

/* Stack sizes */
#define SCREEN_STACK_SIZE     (1024U)
#define BALL_STACK_SIZE       (512U)
#define PONG_MAIN_STACK_SIZE  (1024U)
#define PLAYER_STACK_SIZE     (512U)

/* Sleep values */
#define PLAYER_SLEEP_MS (50U)
#define PAUSE_SLEEP_MS	(200U)
#define BALL_SLEEP_MS		(50U)

typedef enum {
  RUNNING = 0,
  SCORING,
  WAITING,
  NUM_MODES
} game_mode_t;

/* Queue macros */
#define PONG_NUM_MSGS (2U)

#endif
