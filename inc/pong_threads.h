/*
 * Author: Aidan Stanford
 */

#ifndef __PONG_THREADS__
#define __PONG_THREADS__

/* Declarations */
void screen_thread(void);

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

#endif
