/**
 *  Doom Port to the nRF52840 by Nicola Wrachien (next-hack in the comments)
 *
 *  This port is based on the excellent doomhack's GBA Doom Port,
 *  with Kippykip additions.
 *  
 *  Several data structures and functions have been optimized 
 *  to fit in only 256kB RAM of nRF52840 (GBA has 384 kB RAM). 
 *  Z-Depth Light has been restored with almost no RAM consumption!
 *  Tons of speed optimizations have been done, and the game now
 *  runs extremely fast, despite the much higher 3D resolution with
 *  respect to GBA.
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright (C) 2021 Nicola Wrachien (next-hack in the comments)
 *  on nRF52840 port.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 * next-hack: packed (24) and 16 bit pointers utility functions and other stuff
 */

#ifndef DOOM_INCLUDE_I_MEMORY_H_
#define DOOM_INCLUDE_I_MEMORY_H_
#include <stdint.h>

#include <string.h>

#include "user_flash.h"

// #define RAM_PTR_BASE 0x20000000UL
// #define EXT_FLASH_BASE 0x12000000
// #define FLASH_PTR_BASE 0

#define RAM_PTR_BASE USER_RAM_BASE_ADDRESS
#define EXT_FLASH_BASE USER_WAD_PARTITION_BASE_ADDRESS
#define FLASH_PTR_BASE USER_FLASH_BASE_ADDRESS
#define PACKED_MEMORY_ADDRESS0 RAM_PTR_BASE
#define PACKED_MEMORY_ADDRESS1 RAM_PTR_BASE
#define PACKED_MEMORY_ADDRESS2 EXT_FLASH_BASE
#define PACKED_MEMORY_ADDRESS3 USER_CACHE_PARTITION_BASE_ADDRESS
//
// #define WAD_ADDRESS (EXT_FLASH_BASE + 4)

#define WAD_ADDRESS (EXT_FLASH_BASE + 4)
// #define FLASH_CODE_KB 268

#define FLASH_CODE_KB USER_CODE_PARTITION_SIZE_KB
// #define FLASH_ADDRESS (0x00000000 + FLASH_CODE_KB * 1024)

// #define FLASH_ADDRESS (FLASH_PTR_BASE + FLASH_CODE_KB * 1024)

#define FLASH_ADDRESS USER_CACHE_PARTITION_BASE_ADDRESS
#define FLASH_IMMUTABLE_REGION_ADDRESS FLASH_ADDRESS
#define FLASH_IMMUTABLE_REGION 0
#define FLASH_LEVEL_REGION 1
// #define FLASH_CACHE_REGION_SIZE ((1024 - FLASH_CODE_KB) * 1024)

#define FLASH_CACHE_REGION_SIZE USER_CACHE_PARTITION_SIZE
// #define FLASH_BLOCK_SIZE 4096

#define FLASH_BLOCK_SIZE USER_FLASH_ERASE_SIZE
//
#define DEBUG_GETSHORTPTR 0
//
#define isOnExternalFlash(a) (((uint32_t) a & EXT_FLASH_BASE) == EXT_FLASH_BASE)

typedef struct
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    uint8_t addrBytes[sizeof(void *)];
#else
    uint8_t addrBytes[3];
#endif
} __attribute((packed)) packedAddress_t;

#ifdef CONFIG_DOOM_NO_COMPACT_PTR
static inline void* getLongPtr(size_t shortPointer)
{ // Special case: short pointer being all 0 => NULL
    if (!shortPointer)
        return 0;

    return (void*) shortPointer;
}

static inline size_t getShortPtr(void *longPtr)
{
    return ((size_t) longPtr);
}
#else
static inline void* getLongPtr(unsigned short shortPointer)
{ // Special case: short pointer being all 0 => NULL
    if (!shortPointer)
        return 0;
    return (void*) (((unsigned int) shortPointer << 2) | RAM_PTR_BASE);
}
static inline unsigned short getShortPtr(void *longPtr)
{
    return ((unsigned int) longPtr) >> 2;
}
#endif // CONFIG_DOOM_NO_COMPACT_PTR

static inline packedAddress_t getPackedAddress(void *addr)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    packedAddress_t ret;
    memcpy(ret.addrBytes, &addr, sizeof(void *));
    return ret;
#else
    packedAddress_t ret;
    unsigned int intaddr = (uint32_t) addr;
    // uint8_t highaddr = intaddr >> 24;
    uint32_t highaddr = intaddr;
    uint8_t highAddrBits = 0;

    if (highaddr >= USER_RAM_BASE_ADDRESS &&
        highaddr < USER_RAM_BASE_ADDRESS + USER_RAM_SIZE)
    {
        intaddr -= USER_RAM_BASE_ADDRESS;
        highAddrBits = 0x0;
    }
    else if (highaddr >= USER_WAD_PARTITION_BASE_ADDRESS &&
             highaddr < USER_WAD_PARTITION_BASE_ADDRESS + USER_WAD_PARTITION_SIZE)
    {
        intaddr -= USER_WAD_PARTITION_BASE_ADDRESS;
        highAddrBits = 0x40;
    }
    else if (highaddr >= USER_CACHE_PARTITION_BASE_ADDRESS &&
             highaddr < USER_CACHE_PARTITION_BASE_ADDRESS + USER_CACHE_PARTITION_SIZE)
    {
        intaddr -= USER_CACHE_PARTITION_BASE_ADDRESS;
        highAddrBits = 0x80;
    }
    else
    {
        k_panic();
    }
    ret.addrBytes[0] = intaddr;
    ret.addrBytes[1] = intaddr >> 8;
    ret.addrBytes[2] = ((intaddr >> 16) & 0x3F) | highAddrBits;
    // LOG_INF("packed %08x to %02x%02x%02x", (uint32_t) addr, ret.addrBytes[0], ret.addrBytes[1], ret.addrBytes[2]);
    // k_busy_wait(20 * 1000);

    return ret;
#endif // CONFIG_DOOM_NO_COMPACT_PTR
}

// static void* unpackAddress(volatile packedAddress_t a) // passed by copy, because it is shorted than a pointer!
static inline void* unpackAddress(packedAddress_t a) // passed by copy, because it is shorted than a pointer!
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    // void *ret;
    uint32_t ret;
    memcpy(&ret, a.addrBytes, sizeof(void *));
    return (void*)ret;
#else
    const uint32_t addresses[] =
    {
        PACKED_MEMORY_ADDRESS0, 
        // PACKED_MEMORY_ADDRESS1, 
        PACKED_MEMORY_ADDRESS2, 
        PACKED_MEMORY_ADDRESS3
    };

    if (!a.addrBytes[2] && !a.addrBytes[1] && !a.addrBytes[0])
    {
        return NULL;
    }
    // volatile uint32_t temp = (addresses[a.addrBytes[2] >> 6] | (a.addrBytes[0]) | (a.addrBytes[1] << 8) | ((a.addrBytes[2] & 0x3F) << 16));
    // volatile uint32_t temp = 0;
    // temp |= a.addrBytes[0];
    // temp |= ((uint32_t) a.addrBytes[1] << 8);
    // temp |= ((uint32_t) (a.addrBytes[2] & 0x3f) << 16);
    // temp += addresses[a.addrBytes[2] >> 6];
    // uint32_t val = temp;
    // return (void *) val;
    return (void*) (addresses[a.addrBytes[2] >> 6] + ((a.addrBytes[0]) | (a.addrBytes[1] << 8) | ((a.addrBytes[2] & 0x3F) << 16)));
#endif // CONFIG_DOOM_NO_COMPACT_PTR
}

#endif /* DOOM_INCLUDE_I_MEMORY_H_ */
