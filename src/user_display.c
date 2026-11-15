#include <zephyr/kernel.h>

#include <zephyr/drivers/display.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/display/cfb.h>

#include "user_display.h"
#include "user_test_images.h"

#if defined(CONFIG_SOC_SERIES_ESP32P4)
#include <esp_cache.h>
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(user_main, LOG_LEVEL_INF);

K_SEM_DEFINE(user_display_sem, 1, 1);

/* The device frames are written to: the zephyr,display chosen when the board
 * defines one (P4: the DSI DPI scanout device; the display-ctrl alias there
 * names the panel for its geometry), otherwise the display-ctrl alias. */
#if DT_HAS_CHOSEN(zephyr_display)
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
#else
static const struct device *display_dev = DEVICE_DT_GET(DT_ALIAS(display_ctrl));
#endif
/* PWM backlight only where a ledc node is wired up (S3/classic boards). On
 * boards whose panel driver owns the backlight GPIO (P4 ST7701) there is no
 * enabled ledc0 node. */
#if !defined(CONFIG_BOARD_NATIVE_SIM) && DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(ledc0))
#define USER_DISPLAY_HAS_PWM_BL 1
static const struct device *pwm_dev = DEVICE_DT_GET(DT_NODELABEL(ledc0));
#else
#define USER_DISPLAY_HAS_PWM_BL 0
#endif

__unused
static void user_display_swap_bytes(uint16_t *buff, size_t len)
{
	static uint8_t swapped = 0;

	if (buff == NULL || len == 0 || swapped != 0) { return; }
	
	//color correction
	for (size_t i = 0, temp = 0; i < len; i++)
	{
		temp = buff[i];
		buff[i] = buff[i] >> 8;
		buff[i] = buff[i] | ((temp & 0x00ff) << 8);
	}

	swapped = 1;
}

void user_display_image_rotate(uint16_t *buff, size_t len)
{
	if (buff == NULL || len == 0) { return; }

	for (size_t i = 0, j = len - 1; i < j; i++, j--)
	{
		size_t temp = buff[i];
		buff[i] = buff[j];
		buff[j] = temp;
	}
}

void user_display_image_mirror(uint16_t *buff, size_t width, size_t height)
{
	if (buff == NULL || height == 0 || width == 0) { return; }

	for (size_t i = 0; i < width; i++)
	{
		for (size_t j = 0; j < height / 2; j++)
		{
			uint16_t temp = buff[i * height + j];
			buff[i * height + j] = buff[i * height + (height - 1 - j)];
			buff[i * height + (height - 1 - j)] = temp;
		}

		if (height % 2 != 0)
		{
			buff[i * height + (height / 2)] = buff[i * height + (height / 2)];
		}
	}
}

int user_display_test_image(void)
{
	int ret = 0;

	const uint32_t display_buff_len    = ARRAY_SIZE(image_bird);

	const struct display_buffer_descriptor display_buff_conf =
	{
		.buf_size = display_buff_len,
		//important to inverse h and w for proper drawing
		.height   = USER_SCREEN_WIDTH, 
		.width    = USER_SCREEN_HEIGHT,
		.pitch    = 2,
	};

	uint16_t *buff = image_bird;
	const uint16_t height = USER_SCREEN_HEIGHT;
	const uint16_t width = USER_SCREEN_WIDTH;
	
	static uint32_t counter = 0;

	// for (size_t row = 0; row < height; row++)
	// {
	// 	for (size_t col = 0; col < width; col++)
	// 	{
	// 		if (row % 16 == 0)
	// 		{
	// 			buff[row * width + col] = 0xffff;
	// 		}
	// 		else if (col % 15 == 0)
	// 		{
	// 			buff[row * width + col] = 0xaaff;
	// 		}
	// 		else
	// 		{
	// 			buff[row * width + col] = 0x0;
	// 		}
	// 	}
	// }

	if (counter++ % 2 == 0)
	{
		user_display_image_rotate(buff, width * height);
	}
	else
	{
		user_display_image_mirror(buff, width, height);
	}

	ret = display_write(display_dev, 0, 0, &display_buff_conf, buff);
	if (ret != 0) { LOG_INF("display write error %d", ret); }

	return ret;
}

#include <zephyr/sys/byteorder.h>
int user_display_write(uint16_t *buff, uint16_t len)
{
	if (buff == NULL || len == 0) { return -EINVAL; }

	int ret = 0;

	// const  uint16_t red      = (0xf800);
	// const  uint16_t green    = (0x07e0);
	// const  uint16_t blue     = (0x001f);
	// static uint16_t colors[] = 
	// { 
	// 	red, 
	// 	green,
	// 	blue
	// };
	
	// // Write 10x10 pixel blocks instead of individual pixels
	// #define BLOCK_SIZE 10
	// static uint16_t pixel_block[BLOCK_SIZE * BLOCK_SIZE];
	
	// struct display_buffer_descriptor solid_color_buff_conf =
	// {
	// 	.buf_size = sizeof(pixel_block),
	// 	.height   = BLOCK_SIZE,
	// 	.width    = BLOCK_SIZE,
	// 	.pitch    = BLOCK_SIZE,
	// };

	// buff = &colors[0];

	// for (size_t y = 0; y < 170; y += BLOCK_SIZE)
	// {
	// 	for (size_t x = 0; x < 320; x += BLOCK_SIZE)
	// 	{
	// 		// Determine color based on 30-pixel bands
	// 		size_t band = (x / 30) % ARRAY_SIZE(colors);
	// 		uint16_t block_color = 0;
			
	// 		// Fill the entire block with the same color
	// 		for (size_t i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
	// 			pixel_block[i] = block_color;
	// 		}
			
	// 		ret = display_write(display_dev, x, y, &solid_color_buff_conf, pixel_block);
	// 		if (ret != 0 ) { LOG_INF("display_write error %d", ret); }

	// 		k_busy_wait(1000);
	// 	}
	// }

	// while(1)
	// {
	// 	for (size_t y = 0; y < 170; y += BLOCK_SIZE)
	// 	{
	// 		for (size_t x = 0; x < 320; x += BLOCK_SIZE)
	// 		{
	// 			// Determine color based on 30-pixel bands
	// 			size_t band = (x / 30) % ARRAY_SIZE(colors);
	// 			uint16_t block_color = colors[band];
				
	// 			// Fill the entire block with the same color
	// 			for (size_t i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
	// 				pixel_block[i] = block_color;
	// 			}
				
	// 			ret = display_write(display_dev, x, y, &solid_color_buff_conf, pixel_block);
	// 			if (ret != 0 ) { LOG_INF("display_write error %d", ret); }

	// 			k_busy_wait(1000 * 10);
	// 		}
	// 	}

	// 	uint16_t temp = colors[0];
	// 	colors[0] = colors[1];
	// 	colors[1] = colors[2];
	// 	colors[2] = temp;
	// }
	


	
	
	
	
	

#if defined(CONFIG_SOC_SERIES_ESP32P4)
	/* The game renders landscape 320x170; the panel scans out portrait
	 * 480x800 from a PSRAM framebuffer the DPI driver owns. Rotate 90
	 * degrees and pixel-double straight into that framebuffer:
	 *   game x (0..319) -> panel y (80..720), game y (0..169) -> panel x
	 *   (70..410), 2x2 blocks, centered. One cache flush per full frame.
	 */
	uint16_t *fb = display_get_framebuffer(display_dev);

	if (fb == NULL) { return -ENODEV; }
	/* Write through the non-cacheable PSRAM alias (+0x40000000): every store
	 * goes straight to PSRAM, so no cache-line flush bursts collide with the
	 * scanout DMA (rev<3.0 corrupts DMA reads that overlap bursty writes -
	 * the "green sparkle" on heavy-update frames), and no msync is needed. */
	fb = (uint16_t *)((uintptr_t)fb + 0x40000000u);

	static uint16_t gy0; /* first game row carried by this chunk */
	const size_t pixels = len / 2;
	const uint16_t xoff = (USER_SCREEN_PHYSICAL_WIDTH - 2 * USER_SCREEN_HEIGHT) / 2;
	const uint16_t yoff = (USER_SCREEN_PHYSICAL_HEIGHT - 2 * USER_SCREEN_WIDTH) / 2;

	for (size_t i = 0; i < pixels; i++)
	{
		const uint16_t gx = i % USER_SCREEN_WIDTH;
		const uint16_t gy = gy0 + i / USER_SCREEN_WIDTH;
		const uint16_t c = buff[i];
		const uint32_t px = xoff + 2 * (USER_SCREEN_HEIGHT - 1 - gy);
		const uint32_t py = yoff + 2 * gx;
		uint16_t *dst = &fb[py * USER_SCREEN_PHYSICAL_WIDTH + px];

		dst[0] = c;
		dst[1] = c;
		dst[USER_SCREEN_PHYSICAL_WIDTH] = c;
		dst[USER_SCREEN_PHYSICAL_WIDTH + 1] = c;
	}

	gy0 = (gy0 + pixels / USER_SCREEN_WIDTH) % USER_SCREEN_HEIGHT;

	return 0;
#else
	struct display_buffer_descriptor conf =
	{
		.buf_size = len,
		.height   = (len / 2) / USER_SCREEN_WIDTH,
		.width    = USER_SCREEN_WIDTH,
		.pitch    = USER_SCREEN_WIDTH,
	};

	static uint16_t x = 0, y = 0;

	ret = display_write(display_dev, x, y, &conf, buff);
	if (ret != 0 ) { LOG_INF("display_write error %d", ret); }

	//divide due to 2 bytes of color per pixel
	y = (y + (x + (conf.buf_size / 2)) / USER_SCREEN_WIDTH) % USER_SCREEN_HEIGHT;
	x = (x + (conf.buf_size / 2)) % USER_SCREEN_WIDTH;

	return ret;
#endif
}

int user_display_init(void)
{
	int ret = 0;

#if USER_DISPLAY_HAS_PWM_BL
	/* Check if PWM device is ready */
	if (!device_is_ready(pwm_dev)) {
		LOG_INF("PWM device not ready");
		return -EBUSY;
	}
#endif

	ret = device_is_ready(display_dev);
	if (ret == 0)
	{
		LOG_INF("DISPLAY DEV NOT READY");
		return -EBUSY;
	}
	
	ret = display_blanking_off(display_dev);
	if (ret)
	{
		LOG_INF("BLANKING OFF ERR %d ", ret);
		return ret;
	}

#if USER_DISPLAY_HAS_PWM_BL
	/* Set backlight to 25% brightness - PWM period 1ms, duty 250us */
	ret = pwm_set(pwm_dev, 0, PWM_USEC(1000), PWM_USEC(1000), PWM_POLARITY_NORMAL);
	if (ret) {
		LOG_INF("PWM backlight set failed: %d", ret);
		return ret;
	}
#endif

#ifdef CONFIG_DOOM_DISPLAY_PADDING
	//blank padding rows in case height is not even
	if (USER_SCREEN_HEIGHT < USER_SCREEN_PHYSICAL_HEIGHT)
	{
		uint8_t blank[USER_SCREEN_WIDTH * USER_SCREEN_PIXEL_SIZE] = { 0 };
		struct display_buffer_descriptor desc = 
		{
			.buf_size = sizeof(blank),
			.height = 1,
			.width = USER_SCREEN_WIDTH,
		};

		for (uint8_t i = 0; i < USER_SCREEN_PHYSICAL_HEIGHT - USER_SCREEN_HEIGHT; i++)
		{
			ret = display_write(display_dev, 0, USER_SCREEN_HEIGHT + i, &desc, blank);
			if (ret < 0)
			{
				LOG_INF("failed to clear padding rows %d", ret);
				return ret;
			}
		}
	}
#endif

	return ret;
}
