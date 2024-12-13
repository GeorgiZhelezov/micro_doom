#include <zephyr/kernel.h>

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

#include <zephyr/devicetree.h>

#include "user_flash.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(user_main, LOG_LEVEL_INF);

static const struct device *wad_partition_device = DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_WAD_PARTITION_NODE));

static inline uint32_t eval_partition_addr(const uint8_t partition_id)
{
	switch (partition_id)
	{
		case USER_WAD_PARTITION_ID:
			return USER_WAD_PARTITION_BASE_ADDRESS;
			break;
		case USER_CACHE_PARTITION_ID:
			return USER_CACHE_PARTITION_BASE_ADDRESS;
			break;
		case USER_GAME_SETTINGS_PARTITION_ID:
			return USER_GAME_SETTINGS_PARTITION_BASE_ADDRESS;
			break;
		case USER_GAME_SAVES_PARTITION_ID:
			return USER_GAME_SAVES_PARTITION_BASE_ADDRESS;
			break;
		default:
			return -EINVAL;
			break;
	}
}

int user_flash_read(void *buff, size_t len, uint32_t addr, const uint8_t partition_id)
{
	if (buff == NULL || len == 0) { return -EINVAL; }

	int ret;

	const struct flash_area *fa = NULL;
	ret = flash_area_open(partition_id, &fa);
	if (fa == NULL || ret)
	{
		LOG_INF("could not open partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
		}
		goto exit;
	}

	// LOG_PRINTK("read id:%d at %08x ", partition_id, addr);
	addr -= eval_partition_addr(partition_id);
	// LOG_PRINTK("eval %08x\r\n", addr);

	ret = flash_area_read(fa, addr, buff, len);
	if (ret < 0)
	{
		LOG_INF("could not read partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
			k_panic();
		}
		goto exit;
	}

exit:
	flash_area_close(fa);
	return ret;
}

int user_flash_write(void *buff, size_t len, uint32_t addr, const uint8_t partition_id)
{
	if (buff == NULL || len == 0) { return -EINVAL; }

	int ret;

	const struct flash_area *fa = NULL;
	ret = flash_area_open(partition_id, &fa);
	if (fa == NULL || ret)
	{
		LOG_INF("could not open partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
		}
		goto exit;
	}

	// LOG_PRINTK("write id:%d at %08x ", partition_id, addr);
	addr -= eval_partition_addr(partition_id);
	// LOG_PRINTK("eval %08x\r\n", addr);

	ret = flash_area_write(fa, addr, buff, len);
	if (ret < 0)
	{
		LOG_INF("could not write partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
		}
		goto exit;
	}

exit:
	flash_area_close(fa);
	return ret;
}

int user_flash_erase_page(uint32_t addr, const uint8_t partition_id)
{
	int ret;
	
	const struct flash_area *fa = NULL;
	ret = flash_area_open(partition_id, &fa);
	if (fa == NULL || ret)
	{
		LOG_INF("could open partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
		}
		goto exit;
	}

	addr -= eval_partition_addr(partition_id);

	ret = flash_area_erase(fa, addr, USER_FLASH_ERASE_SIZE);
	if(ret < 0)
	{
		LOG_INF("could not erase partition %d page 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
		}
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
	if (ret == 0) { LOG_INF("flash not ready"); return -EBUSY; }

	const struct flash_area *fa = NULL;

	//just for testing
	ret = flash_area_open(USER_CACHE_PARTITION_ID, &fa);
	if (ret < 0) { LOG_INF("could not open cache for reset"); }
	ret = flash_area_erase(fa, 0, USER_CACHE_PARTITION_SIZE);
	if (ret < 0) { LOG_INF("could not erase cache for reset"); }

	ret = flash_area_open(USER_GAME_SAVES_PARTITION_ID, &fa);
	if (ret < 0) { LOG_INF("could not open cache for reset"); }
	ret = flash_area_erase(fa, 0, USER_GAME_SAVES_PARTITION_SIZE);
	if (ret < 0) { LOG_INF("could not erase cache for reset"); }

	ret = flash_area_open(USER_GAME_SETTINGS_PARTITION_ID, &fa);
	if (ret < 0) { LOG_INF("could not open cache for reset"); }
	ret = flash_area_erase(fa, 0, USER_GAME_SETTINGS_PARTITION_SIZE);
	if (ret < 0) { LOG_INF("could not erase cache for reset"); }

	return ret;
}