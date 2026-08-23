/*
 * Author: Aidan Stanford
 */

#ifndef __PONG_THREADS__
#define __PONG_THREADS__

#include <stdbool.h>

/* Declarations */
void screen_thread(void *a, void *b);

void player_thread(void);

void ball_thread(void);

/* Priorities */
#define SCREEN_PRIORITY 4
#define PLAYER_PRIORITY 7
#define BALL_PRIORITY   7
#define MAIN_PRIORITY		5

/* Stack sizes */
#define SCREEN_STACK_SIZE (512U)
#define BALL_STACK_SIZE   (512U)
#define MAIN_STACK_SIZE		(512U)
#define PLAYER_STACK_SIZE (512U)

/* Sleep values */
#define PLAYER_SLEEP_MS (50U)
#define PAUSE_SLEEP_MS	(200U)
#define BALL_SLEEP_MS		(50U)

/* Queue macros */
#define PONG_NUM_MSGS (10U)

#endif
