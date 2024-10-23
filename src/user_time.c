#include <zephyr/kernel.h>
#include <zephyr/posix/time.h>

uint32_t user_get_time_us(void)
{
	struct timespec ts = { 0 };
	
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}