#include <zephyr/kernel.h>

#include <zephyr/drivers/display.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "user_test_images.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_display, LOG_LEVEL_INF);

K_SEM_DEFINE(sem_display, 1, 1);

static const struct device *display_dev         = DEVICE_DT_GET(DT_ALIAS(display_ctrl));
static const struct gpio_dt_spec display_pin_bl = GPIO_DT_SPEC_GET(DT_NODELABEL(display_bl), gpios);

uint16_t *test_image             = image_bird;
const uint32_t test_image_len    = ARRAY_SIZE(image_troll);

const struct display_buffer_descriptor display_buff =
{
	.buf_size = test_image_len,
	.height   = 240,
	.width    = 135,
	.pitch    = 1,
};

static void user_display_swap_bytes(uint16_t *buff, size_t len)
{
	//color correction
	for (size_t i = 0, temp = 0; i < len; i++)
	{
		temp = buff[i];
		buff[i] = buff[i] >> 8;
		buff[i] = buff[i] | ((temp & 0x00ff) << 8);
	}
}

void user_display_image_rotate(void)
{
	for (size_t i = 0, j = test_image_len - 1; i < j; i++, j--)
	{
		size_t temp = test_image[i];
		test_image[i] = test_image[j];
		test_image[j] = temp;
	}
}

void user_display_image_mirror()
{
	for (size_t i = 0; i < display_buff.height; i++)
	{
		for (size_t j = 0; j < display_buff.width / 2; j++)
		{
			uint16_t temp = test_image[i * display_buff.width + j];
			test_image[i * display_buff.width + j] = test_image[i * display_buff.width + (display_buff.width - 1 - j)];
			test_image[i * display_buff.width + (display_buff.width - 1 - j)] = temp;
		}

		if (display_buff.width % 2 != 0)
		{
			test_image[i * display_buff.width + (display_buff.width / 2)] = test_image[i * display_buff.width + (display_buff.width / 2)];
		}
	}
}

void user_display_test_image(void)
{
	for (size_t row = 0; row < display_buff.height; row++)
	{
		for (size_t col = 0; col < display_buff.width; col++)
		{
			if (row && row % 16 == 0)
			{
				// test_image[row * display_buff.width + col] = 0xffff;
			} 
			else if (col && col % 15 == 0)
			{
				// test_image[row * display_buff.width + col] = 0xaaff;
			}
			else
			{
				// test_image[row * display_buff.width + col] = 0x0;
			}
		}
	}

	static uint32_t counter = 0;
	if (counter++ % 2 == 0)
	{
		user_display_image_rotate();
	}
	else
	{
		user_display_image_mirror();
	}

	display_write(display_dev, 0, 0, &display_buff, test_image);
}

void user_display_init(void)
{
	int ret = 0;
	gpio_pin_configure_dt(&display_pin_bl, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set_dt(&display_pin_bl, 0);

	if (!device_is_ready(display_dev))
	{
		LOG_INF("DISPLAY DEV NOT READY");
		return;
	}
	
	ret = display_blanking_off(display_dev);
	if (ret)
	{
		LOG_INF("BLANKING OFF ERR %d ", ret);
	}

	gpio_pin_set_dt(&display_pin_bl, 1);

	user_display_swap_bytes(test_image, test_image_len);
	// memset(test_image, 0, test_image_len * sizeof(test_image[0]));
}