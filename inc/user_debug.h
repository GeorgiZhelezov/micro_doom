#ifndef _USER_DEBUG_H
#define _USER_DEBUG_H

void doomk(const char *fmt, ...);
#define debugi(...)
// #define debugi(...) doomk(__VA_ARGS__)

extern volatile uint32_t global_doom_debug_indexer;
static inline uint32_t doom_debug_indexer(void)
{
	static volatile uint32_t index = 0;
	global_doom_debug_indexer = index;
	return index++;
}

#define doom_debug(format, ...) printk("%d:" format "\n", doom_debug_indexer(), ##__VA_ARGS__);

#endif