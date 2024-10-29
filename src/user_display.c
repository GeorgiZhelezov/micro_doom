#include <zephyr/kernel.h>

#include <zephyr/drivers/display.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/display/cfb.h>

#include "user_display.h"
#include "user_test_images.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(user_main, LOG_LEVEL_INF);

K_SEM_DEFINE(user_display_sem, 1, 1);

static const struct device *display_dev         = DEVICE_DT_GET(DT_ALIAS(display_ctrl));
static const struct gpio_dt_spec display_pin_bl = GPIO_DT_SPEC_GET(DT_NODELABEL(display_bl), gpios);

// uint16_t gpu_buff[240][135];
// uint16_t *display_buff             = &gpu_buff[0][0];
// const uint32_t display_buff_len    = sizeof(gpu_buff) / 2;

uint16_t *display_buff             = image_bird;
const uint32_t display_buff_len    = ARRAY_SIZE(image_bird);

const struct display_buffer_descriptor display_buff_conf =
{
	.buf_size = display_buff_len,
	//important to inverse h and w for proper drawing
	.height   = USER_SCREEN_WIDTH, 
	.width    = USER_SCREEN_HEIGHT,
	.pitch    = 1,
};

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

	uint16_t *buff = display_buff;
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

int user_display_write(uint16_t *buff, uint16_t len)
{
	if (buff == NULL || len == 0) { return -EINVAL; }

	int ret = 0;
	
	struct display_buffer_descriptor conf =
	{
		.buf_size = len,
		.height   = USER_SCREEN_HEIGHT, 
		.width    = USER_SCREEN_WIDTH,
		.pitch    = 1,
	};

	static uint16_t x = 0, y = 0;

	// const uint16_t black = 0x0;
	// const uint16_t yellow = 0xFFE0;
	// const uint16_t green = 0x07E0;
	// const uint16_t red = 0xF800;
	// const uint16_t blue = 0x001F;
	// const uint16_t cyan = 0x07FF;
	// uint8_t palette_counter = 0;
	// uint16_t palette[] = { red, green };
	// uint16_t colors[30] = { 0 };
	// buff = colors;

	// while(1)
	// {
		// for (size_t i = 0; i < ARRAY_SIZE(colors); i++)
		// {
		// 	colors[i] = palette[palette_counter];
		// }
		// palette_counter = (palette_counter + 1 ) % ARRAY_SIZE(palette);
		// conf.buf_size = sizeof(colors);

		ret = display_write(display_dev, x, y, &conf, buff);
		if (ret != 0 ) { LOG_INF("display_write error %d", ret); }

		y = (y + (x + (conf.buf_size / 2)) / conf.width) % conf.height;
		x = (x + (conf.buf_size / 2)) % conf.width;

	// 	k_busy_wait(100 * 1000);
	// }


	return ret;
}

int user_display_init(void)
{
	int ret = 0;
	gpio_pin_configure_dt(&display_pin_bl, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set_dt(&display_pin_bl, 0);

	ret = device_is_ready(display_dev);
	if (ret == 0)
	{
		LOG_INF("DISPLAY DEV NOT READY");
		return ret;
	}
	
	ret = display_blanking_off(display_dev);
	if (ret)
	{
		LOG_INF("BLANKING OFF ERR %d ", ret);
		return ret;
	}

	gpio_pin_set_dt(&display_pin_bl, 1);
	
	return ret;
}