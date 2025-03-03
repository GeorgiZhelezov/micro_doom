#ifndef _USER_CONTROLS_H
#define _USER_CONTROLS_H

/**
 * @brief Get the state of the keyboard pins
 * 
 * @return
 * - 0 on success
 * 
 * - negative on failure
 */
int user_contorls_get_state(uint16_t *keys);

/**
 * @brief Controls initialization
 * 
 * @return
 * - 0 on success
 * 
 * - negative on failure
 */
int user_controls_init(void);

#endif