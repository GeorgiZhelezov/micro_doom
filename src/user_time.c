#include <zephyr/kernel.h>
#include <zephyr/posix/time.h>

uint32_t user_get_time(void)
{
	struct timespec ts = { 0 };
	
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_nsec / 1000 + ts.tv_nsec * 1000;
}