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
 * next-hack: support for patches i external flash
 *
 *-----------------------------------------------------------------------------*/

#include "z_zone.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_sky.h"
#include "r_things.h"
#include "p_tick.h"
#include "i_system.h"
#include "r_draw.h"
#include "lprintf.h"
#include "r_patch.h"
#include <assert.h>
#include "i_spi_support.h"
#include "global_data.h"
#include "r_patch.h"
#include "i_memory.h"

//---------------------------------------------------------------------------
int R_NumPatchWidth(int lump)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    uint32_t temp_offsets_addr;
    user_flash_read_game_resource(&temp_offsets_addr, sizeof(temp_offsets_addr), (uint32_t)&p_wad_immutable_flash_data->patchLumpSizeOffsets);
    temp_offsets_addr = (uint32_t)((patchsizeoffsets_t*)temp_offsets_addr + lump);
    patchsizeoffsets_t temp_offsets;
    user_flash_read_game_resource(&temp_offsets, sizeof(temp_offsets), temp_offsets_addr);
    return temp_offsets.width;
#else
    return p_wad_immutable_flash_data->patchLumpSizeOffsets[lump].width;
#endif
}

//---------------------------------------------------------------------------
int R_NumPatchHeight(int lump)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    uint32_t temp_offsets_addr;
    user_flash_read_game_resource(&temp_offsets_addr, sizeof(temp_offsets_addr), (uint32_t)&p_wad_immutable_flash_data->patchLumpSizeOffsets);
    temp_offsets_addr = (uint32_t)((patchsizeoffsets_t*)temp_offsets_addr + lump);
    patchsizeoffsets_t temp_offsets;
    user_flash_read_game_resource(&temp_offsets, sizeof(temp_offsets), temp_offsets_addr);
    return temp_offsets.height;
#else
    return p_wad_immutable_flash_data->patchLumpSizeOffsets[lump].height;
#endif

}

