/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "esp_attr.h"

#include "user_display.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_main, LOG_LEVEL_INF);

void user_print_device_info(void)
{
    LOG_INF("Doom port to ESP32. Detecting MCU data");
    LOG_INF("RAM: %dKB Flash: %dKB", CONFIG_SRAM_SIZE, CONFIG_FLASH_SIZE / 1000);
}

void user_print_char(char c)
{
	//NOTE wonder what impact a printf over printf causes
	printk("%c", c);
}

void user_delay(uint32_t milis)
{
	k_busy_wait(milis * 1000);
}

extern void main_port(void);

void main(void)
{
	int ret = 0;

	user_display_init();

	user_display_test_image();
	
	while(1)
	{
		LOG_INF("Hello World! %s", CONFIG_BOARD);
		k_sleep(K_MSEC(1000));

		// main_port();
	}
}
