#ifndef INC_USER_TIME_H
#define INC_USER_TIME_H

#include <stdint.h>

#include <zephyr/posix/time.h>

/**
 * @brief Performance profiler macro. See \@details.
 * 
 * @param tag string to print as a tag
 * @details Oddly enough, testing with busy_wait(1us) results in 25us printed, which is weird.
 * 			Then again, it is stated that the clock could be skewed.
 * 			The 20ms busy wait at the end is for the blackmagicprobe RTT to catch up.
 */
#define PERFORMANCE_PROFILE(tag, func)                                           \
	do                                                                           \
	{                                                                            \
		char *t = tag;                                                           \
		if (t == NULL) { t = ""; }                                               \
		uint64_t cycles_now, cycles_after;                                       \
		cycles_now = k_cycle_get_64();                                           \
		func;                                                                    \
		cycles_after = k_cycle_get_64();                                         \
		uint64_t delta_cycles = cycles_after - cycles_now;                       \
		uint64_t delta_ns =                                                      \
			(delta_cycles * 1000000000ULL) / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC; \
		uint64_t delta_us = delta_ns / 1000ULL;                                  \
		printk("%s %4d:%s took %4llu us (%llu ns)\r\n", t, __LINE__, __func__,   \
			   delta_us,                                                         \
			   delta_ns);                                                        \
		k_busy_wait(20 * 1000);                                                  \
	} while (0);

/**
 * @brief Wrapper for getting the current time in microseconds
 * 
 * @return 
 * - current CLOCK_MONOTONIC time in microseconds 
 */
uint32_t user_get_time_us(void);

/**
 * @brief Wrapper for a blocking delay
 * 
 * @param millis milliseconds to delay
 */
void user_delay(uint32_t millis);

#endif