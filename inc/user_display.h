#ifndef INC_USER_DISPLAY_H
#define INC_USER_DISPLAY_H

#include <zephyr/kernel.h>

extern struct k_sem sem_display;

void user_display_test_image(void);
void user_display_init(void);

#endif // INC_USER_DISPLAY_H