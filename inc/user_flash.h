#ifndef INC_USER_FLASH_H
#define INC_USER_FLASH_H

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>

#define USER_RAM_NODE 			DT_NODELABEL(sram0)
#define USER_RAM_BASE_ADDRESS 	DT_REG_ADDR(USER_RAM_NODE)
#define USER_RAM_SIZE 			DT_REG_SIZE(USER_RAM_NODE)

#define USER_FLASH_NODE 		DT_NODELABEL(flash0)
#define USER_FLASH_BASE_ADDRESS DT_REG_ADDR(USER_FLASH_NODE)
#define USER_FLASH_SIZE 		DT_REG_SIZE(USER_FLASH_NODE)
#define USER_FLASH_WRITE_SIZE 	DT_PROP(USER_FLASH_NODE, write_block_size)
#define USER_FLASH_ERASE_SIZE 	DT_PROP(USER_FLASH_NODE, erase_block_size)

#define USER_WAD_PARTITION_NODE 		DT_NODELABEL(wad_partition)
#define USER_WAD_PARTITION_ID 			DT_FIXED_PARTITION_ID(USER_WAD_PARTITION_NODE)
#define USER_WAD_PARTITION_BASE_ADDRESS (DT_REG_ADDR(USER_WAD_PARTITION_NODE) + USER_FLASH_BASE_ADDRESS)
#define USER_WAD_PARTITION_SIZE 		FIXED_PARTITION_NODE_SIZE(USER_WAD_PARTITION_NODE)
#define USER_WAD_PARTITION_SIZE_KB 		(USER_WAD_PARTITION_SIZE / 1024)

#define USER_CODE_PARTITION_NODE 		 DT_NODELABEL(code_partition)
#define USER_CODE_PARTITION_BASE_ADDRESS (DT_REG_ADDR(USER_CODE_PARTITION_NODE) + USER_FLASH_BASE_ADDRESS)
#define USER_CODE_PARTITION_SIZE 		 DT_REG_SIZE(USER_CODE_PARTITION_NODE)
#define USER_CODE_PARTITION_SIZE_KB 	 (USER_CODE_PARTITION_SIZE / 1024)

#define USER_CACHE_PARTITION_NODE 			DT_NODELABEL(cache_partition)
#define USER_CACHE_PARTITION_ID 			DT_FIXED_PARTITION_ID(USER_CACHE_PARTITION_NODE)
#define USER_CACHE_PARTITION_BASE_ADDRESS 	(DT_REG_ADDR(USER_CACHE_PARTITION_NODE) + USER_FLASH_BASE_ADDRESS)
#define USER_CACHE_PARTITION_SIZE 		 	DT_REG_SIZE(USER_CACHE_PARTITION_NODE)
#define USER_CACHE_PARTITION_SIZE_KB 		(USER_CACHE_PARTITION_SIZE / 1024)

#define USER_GAME_SETTINGS_PARTITION_NODE 			DT_NODELABEL(game_settings_partition)
#define USER_GAME_SETTINGS_PARTITION_ID 			DT_FIXED_PARTITION_ID(USER_GAME_SETTINGS_PARTITION_NODE)
#define USER_GAME_SETTINGS_PARTITION_BASE_ADDRESS 	(DT_REG_ADDR(USER_GAME_SETTINGS_PARTITION_NODE) + USER_FLASH_BASE_ADDRESS)
#define USER_GAME_SETTINGS_PARTITION_SIZE 		 	DT_REG_SIZE(USER_GAME_SETTINGS_PARTITION_NODE)
#define USER_GAME_SETTINGS_PARTITION_SIZE_KB 		(USER_GAME_SETTINGS_PARTITION_SIZE / 1024)

#define USER_GAME_SAVES_PARTITION_NODE 			DT_NODELABEL(game_saves_partition)
#define USER_GAME_SAVES_PARTITION_ID 			DT_FIXED_PARTITION_ID(USER_GAME_SAVES_PARTITION_NODE)
#define USER_GAME_SAVES_PARTITION_BASE_ADDRESS 	(DT_REG_ADDR(USER_GAME_SAVES_PARTITION_NODE) + USER_FLASH_BASE_ADDRESS)
#define USER_GAME_SAVES_PARTITION_SIZE 		 	DT_REG_SIZE(USER_GAME_SAVES_PARTITION_NODE)
#define USER_GAME_SAVES_PARTITION_SIZE_KB 		(USER_GAME_SAVES_PARTITION_SIZE / 1024)

/**
 * @brief Flash initialization
 * @details Checks if the flash device driver is ready
 * 
 * @return 
 * - -EBUSY if busy
 * 
 * - True if not busy
 */
int user_flash_init(void);

/**
 * @brief Read data from flash to a buffer
 * 
 * @note The underlying Zephyr flash API requires the address offset from the base address of the
 * 		 partition, but this function requires the physical address. This is done just so I can figure
 * 		 out where data wants to be read/written/erased. Might change the function later to require
 * 		 just the physical address and offsets/partitions are calculated internally.
 * 
 * @param buff buffer to write flash data to
 * @param len length of buff
 * @param addr address in flash to read data from
 * @param partition_id desired partition to read from
 * @return 
 * - 0 on success
 * 
 * - negative errno on failure
 */
int user_flash_read(void *buff, size_t len, uint32_t addr, const uint8_t partition_id);

/**
 * @brief Write data to flash from a buffer
 * 
 * @note The underlying Zephyr flash API requires the address offset from the base address of the
 * 		 partition, but this function requires the physical address. This is done just so I can figure
 * 		 out where data wants to be read/written/erased. Might change the function later to require
 * 		 just the physical address and offsets/partitions are calculated internally.
 * 
 * @param buff buffer to write to flash 
 * @param len length of buff
 * @param addr address in flash to write data to
 * @param partition_id desired partition to write data to
 * @return
 * - 0 on success
 * 
 * - negative errno on failure
 */
int user_flash_write(void *buff, size_t len, uint32_t addr, const uint8_t partition_id);

/**
 * @brief Erase a whole flash page
 * 
 * @note The underlying Zephyr flash API requires the address offset from the base address of the
 * 		 partition, but this function requires the physical address. This is done just so I can figure
 * 		 out where data wants to be read/written/erased. Might change the function later to require
 * 		 just the physical address and offsets/partitions are calculated internally.
 * 
 * @param addr address of the flash page
 * @param partition_id desired partition to erase a page from
 * @return
 * - 0 on success
 * 
 * - negative errno on failure 
 */
int user_flash_erase_page(uint32_t addr, const uint8_t partition_id);

/**
 * @brief Read a game resource from flash (sides, sectors, lines...)
 * 
 * @note Had enough of copy pasting the same switch case for the same LN_* macros
 * 		so a bit of cleanup is needed
 * 
 * @param buff buffer to write flash data to
 * @param len size of buff
 * @param addr address in flash to read data from
 * @return
 * - 0 on success
 * 
 * - negative errno on failure 
 */
int user_flash_read_game_resource(void *buff, size_t len, uint32_t addr);

/**
 * @brief Figure out if some buffer/game resource is in flash or not
 * 
 * @param addr address of buffer
 * 
 * @return
 * - true if in flash
 * 
 * - false if not in flash 
 */
int user_flash_is_resource_in_flash(uint32_t addr);

#endif