#include <zephyr/kernel.h>

#include <zephyr/drivers/display.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/display/cfb.h>

#include "user_test_images.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_display, LOG_LEVEL_INF);

K_SEM_DEFINE(sem_display, 1, 1);

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
	for (size_t i = 0, j = display_buff_len - 1; i < j; i++, j--)
	{
		size_t temp = display_buff[i];
		display_buff[i] = display_buff[j];
		display_buff[j] = temp;
	}
}

void user_display_image_mirror()
{
	for (size_t i = 0; i < display_buff_conf.height; i++)
	{
		for (size_t j = 0; j < display_buff_conf.width / 2; j++)
		{
			uint16_t temp = display_buff[i * display_buff_conf.width + j];
			display_buff[i * display_buff_conf.width + j] = display_buff[i * display_buff_conf.width + (display_buff_conf.width - 1 - j)];
			display_buff[i * display_buff_conf.width + (display_buff_conf.width - 1 - j)] = temp;
		}

		if (display_buff_conf.width % 2 != 0)
		{
			display_buff[i * display_buff_conf.width + (display_buff_conf.width / 2)] = display_buff[i * display_buff_conf.width + (display_buff_conf.width / 2)];
		}
	}
}

void user_display_test_image(void)
{
	static uint32_t counter = 0;

	for (size_t row = 0; row < display_buff_conf.height; row++)
	{
		for (size_t col = 0; col < display_buff_conf.width; col++)
		{
			if (row % 16 == 0)
			{
				display_buff[row * display_buff_conf.width + col] = 0xffff;
			}
			else if (col % 15 == 0)
			{
				display_buff[row * display_buff_conf.width + col] = 0xaaff;
			}
			else
			{
				display_buff[row * display_buff_conf.width + col] = 0x0;
			}
		}
	}

	if (counter++ % 2 == 0)
	{
		user_display_image_rotate();
	}
	else
	{
		// user_display_image_mirror();
	}

	display_write(display_dev, 0, 0, &display_buff_conf, display_buff);
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

	user_display_swap_bytes(display_buff, display_buff_len);
	// memset(display_buff, 0, display_buff_conf.buf_size * sizeof(display_buff[0]));
}