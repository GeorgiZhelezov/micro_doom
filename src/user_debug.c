#include <stdio.h>
#include <stdint.h>

#include <zephyr/kernel.h>

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
#include <zephyr/posix/time.h>
void printk(const char *fmt, ...)
{
	uint32_t i = indexer();
	// return;

	// if (i < 52000) { return; }

	// if (i == 52925)
	// {
	// 	__unused volatile int a = i;
	// }

	printk_custom("%u: ", i);
	
	va_list ap;

	va_start(ap, fmt);

	vprintk(fmt, ap);

	va_end(ap);
}