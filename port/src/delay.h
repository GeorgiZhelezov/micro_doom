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
 *  DESCRIPTION:
 *  delay utility function.
 *
 */
#ifndef SRC_DELAY_H_
#define SRC_DELAY_H_
#include <stdint.h>
// #include "nrf.h"
#include "user_time.h"

static inline void delay(uint32_t milliseconds)
{
    // milliseconds *= 1000;
    // uint32_t timeNow = NRF_TIMER3->CC[0];
    // while ((uint32_t)NRF_TIMER3->CC[0] - timeNow < milliseconds)
    //     ;
	user_delay(milliseconds);
}
#endif /* SRC_DELAY_H_ */