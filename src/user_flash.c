#include <zephyr/kernel.h>

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

#include <zephyr/devicetree.h>

#include "user_flash.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(user_main, LOG_LEVEL_INF);

static const struct device *wad_partition_device = DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_WAD_PARTITION_NODE));

int user_flash_read(void *buff, size_t len, uint32_t addr)
{
	if (buff == NULL || len == 0) { return -EINVAL; }
	
	int ret;

	const struct flash_area *fa = NULL;
	ret = flash_area_open(USER_WAD_PARTITION_ID, &fa);
	if (fa == NULL || ret)
	{
		LOG_INF("could not open wad [%d]", ret);
		goto exit;
	}

	addr = addr - USER_WAD_PARTITION_BASE_ADDRESS;
	ret = flash_area_read(fa, addr, buff, len);
	if (ret < 0)
	{
		LOG_INF("could read wad [%d]", ret);
		goto exit;
	}

exit:
	flash_area_close(fa);
	return ret;
}

int user_flash_init(void)
{
	int ret;

	ret = device_is_ready(wad_partition_device);
	if (ret == 0)  { LOG_INF("flash not ready"); }

	return ret;
}