#include <zephyr/kernel.h>

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

#include <zephyr/devicetree.h>

#include "user_flash.h"

#ifdef CONFIG_BOARD_NATIVE_SIM
	#ifdef CONFIG_GITHUB_ACTIONS_BUILD
#include "demo.h"
	#else
// #include "demo.h"
#include "game.h"
	#endif
#endif //CONFIG_BOARD_NATIVE_SIM

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(user_main, LOG_LEVEL_INF);

static const struct device *partitions[] =
{
	DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_WAD_PARTITION_NODE)),
	DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_CACHE_PARTITION_NODE)),
	DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_GAME_SETTINGS_PARTITION_NODE)),
	DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_GAME_SAVES_PARTITION_NODE)),
};

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


#if defined(CONFIG_SOC_SERIES_ESP32P4)
/* The esp flash layer can only read/write directly from/to internal DRAM;
 * game buffers may live in PSRAM (or other non-DRAM memory), for which the
 * Zephyr flash OSAL provides no bounce buffer (get_temp_buffer == NULL ->
 * ESP_ERR_NO_MEM). Bounce every access through a small internal-SRAM buffer.
 */
static uint8_t flash_bounce[256];
K_MUTEX_DEFINE(flash_bounce_lock);

static int bounced_flash_read(const struct flash_area *fa, uint32_t addr, void *buff, size_t len)
{
	uint8_t *dst = buff;
	int ret = 0;

	k_mutex_lock(&flash_bounce_lock, K_FOREVER);
	while (len > 0)
	{
		const size_t chunk = MIN(len, sizeof(flash_bounce));

		ret = flash_area_read(fa, addr, flash_bounce, chunk);
		if (ret < 0) { break; }
		memcpy(dst, flash_bounce, chunk);
		dst += chunk;
		addr += chunk;
		len -= chunk;
	}
	k_mutex_unlock(&flash_bounce_lock);
	return ret;
}

static int bounced_flash_write(const struct flash_area *fa, uint32_t addr, const void *buff, size_t len)
{
	const uint8_t *src = buff;
	int ret = 0;

	k_mutex_lock(&flash_bounce_lock, K_FOREVER);
	while (len > 0)
	{
		const size_t chunk = MIN(len, sizeof(flash_bounce));

		memcpy(flash_bounce, src, chunk);
		ret = flash_area_write(fa, addr, flash_bounce, chunk);
		if (ret < 0) { break; }
		src += chunk;
		addr += chunk;
		len -= chunk;
	}
	k_mutex_unlock(&flash_bounce_lock);
	return ret;
}
#else
#define bounced_flash_read(fa, addr, buff, len)  flash_area_read(fa, addr, buff, len)
#define bounced_flash_write(fa, addr, buff, len) flash_area_write(fa, addr, buff, len)
#endif

#if defined(CONFIG_SOC_SERIES_ESP32P4)
/* Memory-map the game's read-heavy flash region (cache + WAD partitions) so
 * lump/resource reads run at XIP speed through the L2 cache instead of one
 * flash-API transaction per access. Writes still go through the flash API;
 * the mapped window is invalidated afterwards so readers see fresh data. */
#include <hal/mmu_hal.h>
#include <hal/mmu_types.h>
#include <esp_cache.h>

#define P4_MAP_PA    0xC0000u                  /* covers cache (0xC2000) + wad (0x22b000+) */
#define P4_MAP_SIZE  (0x1000000u - P4_MAP_PA)  /* up to the 16MB flash end */
#define P4_MAP_VA    0x40400000u

static bool p4_map_ready;

static void p4_flash_map_init(void)
{
	uint32_t mapped = 0;

	mmu_hal_map_region(0, MMU_TARGET_FLASH0, P4_MAP_VA, P4_MAP_PA, P4_MAP_SIZE, &mapped);
	if (mapped >= P4_MAP_SIZE)
	{
		esp_cache_msync((void *)P4_MAP_VA, P4_MAP_SIZE,
				ESP_CACHE_MSYNC_FLAG_DIR_M2C);
		p4_map_ready = true;
		LOG_INF("game flash window mapped: pa 0x%08x -> va 0x%08x (%u KB)",
			P4_MAP_PA, P4_MAP_VA, P4_MAP_SIZE / 1024);
	}
	else
	{
		LOG_INF("game flash map failed (mapped %u of %u)", mapped, P4_MAP_SIZE);
	}
}

static inline bool p4_addr_mapped(uint32_t addr, size_t len)
{
	return p4_map_ready && addr >= P4_MAP_PA && (addr + len) <= (P4_MAP_PA + P4_MAP_SIZE);
}

static inline void p4_map_invalidate(uint32_t addr, size_t len)
{
	if (!p4_addr_mapped(addr, len)) { return; }
	/* align out to cache lines */
	uint32_t start = (P4_MAP_VA + (addr - P4_MAP_PA)) & ~63u;
	uint32_t end = (P4_MAP_VA + (addr - P4_MAP_PA) + len + 63u) & ~63u;

	esp_cache_msync((void *)start, end - start, ESP_CACHE_MSYNC_FLAG_DIR_M2C);
}
#else
#define p4_addr_mapped(addr, len) 0
#define p4_map_invalidate(addr, len)
#endif

int user_flash_read(void *buff, size_t len, uint32_t addr, const uint8_t partition_id)
{
	if (buff == NULL || len == 0) { return -EINVAL; }

	int ret;

#if defined(CONFIG_SOC_SERIES_ESP32P4) && !defined(P4_MAP_DISABLE)
	if (p4_addr_mapped(addr, len))
	{
		memcpy(buff, (const void *)(P4_MAP_VA + (addr - P4_MAP_PA)), len);
		return 0;
	}
#endif

	const struct flash_area *fa = NULL;
	ret = flash_area_open(partition_id, &fa);
	if (fa == NULL || ret)
	{
		LOG_INF("could not open partition %d on 0x%08x [%d]", partition_id, addr, ret);
		while(1)
		{
			arch_nop();
			k_panic();
		}
		goto exit;
	}

	// LOG_PRINTK("read id:%d at %08x ", partition_id, addr);
	addr -= eval_partition_addr(partition_id);
	// LOG_PRINTK("eval %08x\r\n", addr);

	ret = bounced_flash_read(fa, addr, buff, len);
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
			k_panic();
		}
		goto exit;
	}

	// LOG_PRINTK("write id:%d at %08x ", partition_id, addr);
	addr -= eval_partition_addr(partition_id);
	// LOG_PRINTK("eval %08x\r\n", addr);

	ret = bounced_flash_write(fa, addr, buff, len);
	if (ret >= 0)
	{
		p4_map_invalidate(addr + eval_partition_addr(partition_id), len);
	}
	if (ret < 0)
	{
		LOG_INF("could not write partition %d on 0x%08x [%d]", partition_id, addr, ret);
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
			k_panic();
		}
		goto exit;
	}

	addr -= eval_partition_addr(partition_id);

	ret = flash_area_erase(fa, addr, USER_FLASH_ERASE_SIZE);
	if (ret >= 0)
	{
		p4_map_invalidate(addr + eval_partition_addr(partition_id), USER_FLASH_ERASE_SIZE);
	}
	if(ret < 0)
	{
		LOG_INF("could not erase partition %d page 0x%08x [%d]", partition_id, addr, ret);
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

int user_flash_is_resource_in_flash(const uint32_t addr)
{
	if ((addr >= USER_WAD_PARTITION_BASE_ADDRESS &&
		 addr < USER_WAD_PARTITION_BASE_ADDRESS + USER_WAD_PARTITION_SIZE) ||

		(addr >= USER_CACHE_PARTITION_BASE_ADDRESS &&
		 addr < USER_CACHE_PARTITION_BASE_ADDRESS + USER_CACHE_PARTITION_SIZE))
	{
		return 1;
	}
	return 0;
}

int user_flash_read_game_resource(void *buff, size_t len, uint32_t addr)
{
	int ret = 0;

	if (addr >= USER_WAD_PARTITION_BASE_ADDRESS &&
		addr < USER_WAD_PARTITION_BASE_ADDRESS + USER_WAD_PARTITION_SIZE)
	{
		ret = user_flash_read(buff, len, addr, USER_WAD_PARTITION_ID);
	}
	else if (addr >= USER_CACHE_PARTITION_BASE_ADDRESS &&
			 addr < USER_CACHE_PARTITION_BASE_ADDRESS + USER_CACHE_PARTITION_SIZE)
	{
		ret = user_flash_read(buff, len, addr, USER_CACHE_PARTITION_ID);
	}
	else
	{
		k_panic();
	}

	return ret;
}

int user_flash_init(void)
{
	int ret;

#if defined(CONFIG_SOC_SERIES_ESP32P4)
	p4_flash_map_init();
	{
		/* read-path integrity check: sum the first 64KB of the WAD through
		 * the flash API and through the MMU map - must match each other and
		 * the host file. */
		static uint32_t chunk[256/4];
		uint32_t sum_api = 0, sum_map = 0;
		const struct flash_area *fa = NULL;

		if (flash_area_open(USER_WAD_PARTITION_ID, &fa) == 0)
		{
			for (uint32_t off = 0; off < 0x10000; off += sizeof(chunk))
			{
				flash_area_read(fa, off, chunk, sizeof(chunk));
				for (size_t i = 0; i < ARRAY_SIZE(chunk); i++) { sum_api += chunk[i]; }
			}
			flash_area_close(fa);
		}
		if (p4_map_ready)
		{
			const uint32_t *m = (const uint32_t *)(P4_MAP_VA +
				(USER_WAD_PARTITION_BASE_ADDRESS - P4_MAP_PA));
			for (uint32_t i = 0; i < 0x10000 / 4; i++) { sum_map += m[i]; }
		}
		LOG_INF("wad sum64k: api=%08x map=%08x %s", sum_api, sum_map,
			sum_api == sum_map ? "MATCH" : "MISMATCH");
		if (p4_map_ready)
		{
			static const uint32_t deep[] = {0x300000, 0x400000, 0x480000};

			for (size_t d = 0; d < ARRAY_SIZE(deep); d++)
			{
				const uint32_t *m = (const uint32_t *)(P4_MAP_VA +
					(USER_WAD_PARTITION_BASE_ADDRESS + deep[d] - P4_MAP_PA));
				uint32_t sm = 0;

				for (uint32_t i = 0; i < 0x10000 / 4; i++) { sm += m[i]; }
				LOG_INF("wad sum64k @+%06x: map=%08x", deep[d], sm);
			}
		}
	}
#endif

	for (uint8_t i = 0; i < ARRAY_SIZE(partitions); i++)
	{
		ret = device_is_ready(partitions[i]);
		if (ret == 0)
		{
			LOG_INF("flash not ready");
			return -EBUSY;
		}
	}

	//just for testing
	__unused const struct flash_area *fa = NULL;
	// ret = flash_area_open(USER_CACHE_PARTITION_ID, &fa);
	// if (ret < 0) { LOG_INF("could not open cache for reset"); }
	// ret = flash_area_erase(fa, 0, USER_CACHE_PARTITION_SIZE);
	// if (ret < 0) { LOG_INF("could not erase cache for reset"); }

	// ret = flash_area_open(USER_GAME_SAVES_PARTITION_ID, &fa);
	// if (ret < 0) { LOG_INF("could not open cache for reset"); }
	// ret = flash_area_erase(fa, 0, USER_GAME_SAVES_PARTITION_SIZE);
	// if (ret < 0) { LOG_INF("could not erase cache for reset"); }

	// ret = flash_area_open(USER_GAME_SETTINGS_PARTITION_ID, &fa);
	// if (ret < 0) { LOG_INF("could not open cache for reset"); }
	// ret = flash_area_erase(fa, 0, USER_GAME_SETTINGS_PARTITION_SIZE);
	// if (ret < 0) { LOG_INF("could not erase cache for reset"); }

#ifdef CONFIG_BOARD_NATIVE_SIM
	ret = flash_area_open(USER_WAD_PARTITION_ID, &fa);
	if (ret < 0) { LOG_INF("could not open wad for reset"); }
	ret = flash_area_erase(fa, 0, USER_WAD_PARTITION_SIZE);
	if (ret < 0) { LOG_INF("could not erase wad for reset"); }
	ret = user_flash_write(doom_wad, sizeof(doom_wad), USER_WAD_PARTITION_BASE_ADDRESS, USER_WAD_PARTITION_ID);
	if (ret < 0) { LOG_INF("could not write wad partition"); }
#endif

	return ret;
}