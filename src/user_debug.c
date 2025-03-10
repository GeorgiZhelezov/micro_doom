#include <stdio.h>
#include <stdint.h>

#include <zephyr/kernel.h>

#include "user_debug.h"

volatile uint32_t global_doom_debug_indexer;

static volatile uint32_t global_indexer;
static inline uint32_t indexer(void)
{
	static volatile uint32_t index = 0;
	global_indexer = index;
	return index++;
}

void printk_custom(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
}

void doomk(const char *fmt, ...)
{
	uint32_t i = indexer();
	return;
	// if (i < 451100)
	// {
	// 	return;
	// }

	printk_custom("%u: ", i);
	
	// if(i == 451180)
	// {
	// 	__unused volatile uint32_t a = i;
	// }
	
	va_list ap;
	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
}