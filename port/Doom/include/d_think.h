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
 *  MapObj data. Map Objects or mobjs are actors, entities,
 *  thinker, take-your-pick... anything that moves, acts, or
 *  suffers state changes of more or less violent nature.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __D_THINK__
#define __D_THINK__

#ifdef __GNUG__
#pragma interface
#endif
#include "doomdef.h"
/*
 * Experimental stuff.
 * To compile this as "ANSI C with classes"
 *  we will need to handle the various
 *  action functions cleanly.
 */
// killough 11/98: convert back to C instead of C++
typedef void (*actionf_t)();
//typedef  void (*actionf_v)();
//typedef  void (*actionf_p1)( void* );
//typedef  void (*actionf_p2)( void*, void* );

/* Note: In d_deh.c you will find references to these
 * wherever code pointers and function handlers exist
 */
/*
 typedef union
 {
 actionf_p1    acp1;
 actionf_v     acv;
 actionf_p2    acp2;

 } actionf_t;
 */

/* Historically, "think_t" is yet another
 *  function pointer to a routine to handle
 *  an actor.
 */
typedef actionf_t think_t;

/* Doubly linked list of actors. */
typedef struct thinker_s
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t prev_sptr;
    size_t next_sptr;
#else
    //struct thinker_s*   prev;
    unsigned short prev_sptr;
    //struct thinker_s*   next;
    unsigned short next_sptr;
#endif
    think_t function;

} thinker_t;
static inline thinker_t* getThinkerPrev(thinker_t *pthinker)
{
    return (thinker_t*) getLongPtr(pthinker->prev_sptr);
}
static inline thinker_t* getThinkerNext(thinker_t *pthinker)
{
    return (thinker_t*) getLongPtr(pthinker->next_sptr);
}
#endif
