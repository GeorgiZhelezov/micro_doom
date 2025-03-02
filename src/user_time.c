#include <stdint.h>
#include <stdlib.h>

#include <zephyr/kernel.h>

#include <zephyr/posix/time.h>

#ifdef CONFIG_BOARD_NATIVE_SIM
#include "nsi_timer_model.h"
#endif

volatile uint64_t base_time;

uint32_t user_get_time_us(void)
{
#ifdef CONFIG_BOARD_NATIVE_SIM
	uint32_t nsec;
	uint64_t sec;
	hwtimer_get_pseudohost_rtc_time(&nsec, &sec);

	return (sec * 1e6 + nsec / 1e3) - base_time;
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1e6 + ts.tv_nsec / 1e3);
#endif
}

void user_delay(uint32_t millis)
{
	k_busy_wait(millis * 1000);
}

int user_timer_init(void)
{
#ifdef CONFIG_BOARD_NATIVE_SIM
	uint64_t base_sec;
	uint32_t base_nsec;

	hwtimer_get_pseudohost_rtc_time(&base_nsec, &base_sec);
	base_time = base_sec * 1e6 + base_nsec / 1e3;
#endif
	return 0;
}