#ifndef INC_USER_DISPLAY_H
#define INC_USER_DISPLAY_H

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

#define USER_SCREEN_HEIGHT DT_PROP(DT_NODELABEL(st7789), height)
#define USER_SCREEN_WIDTH DT_PROP(DT_NODELABEL(st7789), width)

extern struct k_sem user_display_sem;

void user_display_test_image(void);
void user_display_image_rotate(uint16_t *buff, size_t len);
void user_display_image_mirror(uint16_t *buff, size_t width, size_t height);
void user_display_init(void);

#endif // INC_USER_DISPLAY_H