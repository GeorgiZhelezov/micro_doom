/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/display/cfb.h>
#include <zephyr/drivers/display.h>

#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <main.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_main, LOG_LEVEL_INF);

static const struct device *display_dev = DEVICE_DT_GET(DT_ALIAS(display_ctrl));
static const struct gpio_dt_spec display_pin_bl   = GPIO_DT_SPEC_GET(DT_NODELABEL(display_bl), gpios);

uint16_t *test_image              = image_bird;
const    uint32_t test_image_size = ARRAY_SIZE(image_troll);

const struct display_buffer_descriptor desc =
{
	.buf_size = test_image_size,
	.height   = 240,
	.width    = 135,
	.pitch    = 1,
};

void main(void)
{
	int ret = 0;
	gpio_pin_configure_dt(&display_pin_bl, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set_dt(&display_pin_bl, 0);

	//color correction
	for (size_t i = 0, temp = 0; i < test_image_size; i++)
	{
		temp = test_image[i];
		test_image[i] = test_image[i] >> 8;
		test_image[i] = test_image[i] | ((temp & 0x00ff) << 8);
	}

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

	display_write(display_dev, 0, 0, &desc, test_image);

	gpio_pin_set_dt(&display_pin_bl, 1);
	while(1)
	{
		LOG_INF("Hello World! %s", CONFIG_BOARD);
		k_sleep(K_MSEC(1000));
	}
}
