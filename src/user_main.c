/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "user_display.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_main, LOG_LEVEL_INF);

void main(void)
{
	int ret = 0;

	user_display_init();

	user_display_test_image();
	
	while(1)
	{
		LOG_INF("Hello World! %s", CONFIG_BOARD);
		k_sleep(K_MSEC(1000));
	}
}
