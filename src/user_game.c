#include <zephyr/kernel.h>

#include "user_display.h"

// for native_sim: 
// be mindful of the game thread priority since blanking messed around with in the sdl thread

#define GAME_THREAD_PRIORITY 20

extern void main_port(void); //DOOM main

void game_thread(void *p1, void *p2, void *p3)
{
	user_display_init(); //must be called here after sdl thread is created
	
	while (1)
	{
		main_port();
	}
}

K_THREAD_DEFINE(game_thread_id, 1024, game_thread, NULL, NULL, NULL, GAME_THREAD_PRIORITY, 0, 0);