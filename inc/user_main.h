#ifndef INC_USER_MAIN_H
#define INC_USER_MAIN_H

#include <stdint.h>

/**
 * @brief Print a message that signifies which board the application is running on
 * 
 */
void user_print_device_info(void);

/**
 * @brief Wrapper for printing a single character
 * 
 * @param c character to print
 */
void user_print_char(char c);

#endif //INC_USER_MAIN_H