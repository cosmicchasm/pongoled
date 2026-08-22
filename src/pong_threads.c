/*
 * Author: Aidan Stanford
 *
 * This file contains all the thread definitions for the pong game
 * including the main thread
 */

#include <zephyr/kernel.h>

#include "graphics_common.h"
#include "pong_threads.h"
#include "pong_core.h"

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

// Honestly we could make the screen highest priority and use a message
// queue or something to communicate a screen change to the thread

// this would HAVE to yield/sleep when not used
void screen_thread(void) { }

// this could lowkey (i guess) submit work to a workqueue or something
// with an interrupt... that'd probably be fine
void player_thread(void) { }

// same with this maybe, but i kinda would want a thread for it if we
// want a pretty specific speed
void ball_thread(void) { }
