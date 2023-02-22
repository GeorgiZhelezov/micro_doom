/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/display/ssd16xx.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/display.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_main, LOG_LEVEL_INF);

static const struct device *display_dev = DEVICE_DT_GET(DT_ALIAS(display_ctrl));

void main(void)
{
	int ret = 0;

	if (!device_is_ready(display_dev))
	{
		LOG_INF("DISPLAY DEV NOT READY");
		return;
	}

	ret = display_set_pixel_format(display_dev, PIXEL_FORMAT_MONO10);
	if (ret)
	{
		LOG_INF("FAILED TO SET PIXEL FORMAT %d", ret);
	}

	ret = cfb_framebuffer_init(display_dev);
	if (ret)
	{
		LOG_INF("FRAMEBUFFER INIT FAIL, %d", ret);
	}

	ret = cfb_framebuffer_clear(display_dev, true);
	if (ret)
	{
		LOG_INF("FRAME BUFFER CLEAR ERR %d", ret);
	}

	ret = display_blanking_off(display_dev);
	if (ret)
	{
		LOG_INF("BLANKING OFF ERR %d ", ret);
	}

	uint16_t rows = cfb_get_display_parameter(display_dev, CFB_DISPLAY_ROWS);
	uint8_t ppt = cfb_get_display_parameter(display_dev, CFB_DISPLAY_PPT);
	int num_fonts = cfb_get_numof_fonts(display_dev);

	uint8_t font_width = 0, font_height = 0;
	for (int idx = 0; idx < num_fonts; idx++)
	{
		ret = cfb_get_font_size(display_dev, idx, &font_width, &font_height);
		if (ret)
		{
			LOG_INF("GET FONT SIZE ERR %d", ret);
		}

		LOG_INF("Index[%d] font dimensions %2dx%d", idx, font_width, font_height);
	}

	ret = cfb_framebuffer_set_font(display_dev, 4);
	if (ret)
	{
		LOG_INF("SET FONT ERR %d", ret);
	}

	cfb_framebuffer_invert(display_dev); // white on black
	
	
	while(1)
	{
		LOG_INF("Hello World! %s", CONFIG_BOARD);
		k_sleep(K_MSEC(1000));
	}
}
