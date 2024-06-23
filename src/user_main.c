/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "user_display.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_main, LOG_LEVEL_INF);

extern void main_port(void);

void user_print_char(char c)
{
	LOG_PRINTK("%c", c);
}

void user_delay(uint32_t millis)
{
	k_busy_wait(millis * 1000);
}

void user_print_device_info(void)
{
	LOG_INF("running on board: %s", CONFIG_BOARD);
}

int main(void)
{
	int ret = 0;

	user_display_init();

	while(1)
	{
		LOG_INF("Hello World! %s", CONFIG_BOARD);
		k_sleep(K_MSEC(1000));

		user_display_test_image();

		main_port();
	}

	return ret;
}
