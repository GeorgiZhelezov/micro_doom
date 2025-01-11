#ifndef INC_USER_TIME_H
#define INC_USER_TIME_H

#include <stdint.h>

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