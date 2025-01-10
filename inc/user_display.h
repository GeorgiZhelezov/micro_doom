#ifndef INC_USER_DISPLAY_H
#define INC_USER_DISPLAY_H

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

#define USER_SCREEN_PHYSICAL_HEIGHT (DT_PROP(DT_ALIAS(display_ctrl), height))
#define USER_SCREEN_PHYSICAL_WIDTH (DT_PROP(DT_ALIAS(display_ctrl), width))

#define USER_SCREEN_HEIGHT (USER_SCREEN_PHYSICAL_HEIGHT - (USER_SCREEN_PHYSICAL_HEIGHT % 2))
#define USER_SCREEN_WIDTH (USER_SCREEN_PHYSICAL_WIDTH - (USER_SCREEN_PHYSICAL_WIDTH % 2))

#if defined(CONFIG_ST7789V_RGB565) || defined(CONFIG_ST7789V_BGR565)
#define USER_SCREEN_PIXEL_SIZE 2
#endif

#ifdef CONFIG_ST7789V_RGB888
#define USER_SCREEN_PIXEL_SIZE 3
#endif

extern struct k_sem user_display_sem;

/**
 * @brief Display a test image from a static array used for testing
 * 
 * @return
 * - 0 on success
 * 
 * - negative errno on failure
 */
int user_display_test_image(void);

/**
 * @brief Write data to the display's frame buffer
 * 
 * @note This will write it sequentially and internal relative coordinate offsets will wrap around,
 * 		 but you must consider where in the whole frame you're writing to, because the display's own 
 * 		 counters might not wrap around and you might be writing data into nothingness.
 * 
 * @param buff data to display
 * @param len length of buff
 * @return 
 * - 0 on success
 * 
 * - negative errno on failure
 */
int user_display_write(uint16_t *buff, uint16_t len);

/**
 * @brief Rotate your frame clockwise
 * 
 * @param buff frame buffer
 * @param len length of buff
 */
void user_display_image_rotate(uint16_t *buff, size_t len);

/**
 * @brief Mirror your frame
 * 
 * @param buff frame buffer
 * @param len length of buff
 */
void user_display_image_mirror(uint16_t *buff, size_t width, size_t height);

/**
 * @brief Display initialization
 * 
 * @note Initializes with whatever settings are given in the overlay for offsets, voltages...
 * 		 Also takes care of blanking any padding rows in case the screen height is not even
 * 
 * @return
 * - 0 on success
 * - -EBUSY if display driver is not ready 
 */
int user_display_init(void);

#endif // INC_USER_DISPLAY_H