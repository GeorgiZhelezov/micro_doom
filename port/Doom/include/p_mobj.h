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
 *      Map Objects, MObj, definition and handling.
 *
 *  next-hack: optimzed a lot this, bringing the structure size from 140 b
 *  down to 92 bytes. Added also static_mobj_t, which uses only 44 bytes.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_MOBJ__
#define __P_MOBJ__
// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//

// Call P_SpecialThing when touched.
#define MF_SPECIAL      (unsigned int)(0x0000000000000001)
// Blocks.
#define MF_SOLID        (unsigned int)(0x0000000000000002)
// Can be hit.
#define MF_SHOOTABLE    (unsigned int)(0x0000000000000004)
// Don't use the sector links (invisible but touchable).
#define MF_NOSECTOR     (unsigned int)(0x0000000000000008)
// Don't use the blocklinks (inert but displayable)
#define MF_NOBLOCKMAP   (unsigned int)(0x0000000000000010)

// Not to be activated by sound, deaf monster.
#define MF_AMBUSH       (unsigned int)(0x0000000000000020)
// Will try to attack right back.
#define MF_JUSTHIT      (unsigned int)(0x0000000000000040)
// Will take at least one step before attacking.
#define MF_JUSTATTACKED (unsigned int)(0x0000000000000080)
// On level spawning (initial position),
//  hang from ceiling instead of stand on floor.
#define MF_SPAWNCEILING (unsigned int)(0x0000000000000100)
// Don't apply gravity (every tic),
//  that is, object will float, keeping current height
//  or changing it actively.
#define MF_NOGRAVITY    (unsigned int)(0x0000000000000200)

// Movement flags.
// This allows jumps from high places.
#define MF_DROPOFF      (unsigned int)(0x0000000000000400)
// For players, will pick up items.
#define MF_PICKUP       (unsigned int)(0x0000000000000800)
// Player cheat. ???
#define MF_NOCLIP       (unsigned int)(0x0000000000001000)
// Player: keep info about sliding along walls.
#define MF_SLIDE        (unsigned int)(0x0000000000002000)
// Allow moves to any height, no gravity.
// For active floaters, e.g. cacodemons, pain elementals.
#define MF_FLOAT        (unsigned int)(0x0000000000004000)
// Don't cross lines
//   ??? or look at heights on teleport.
#define MF_TELEPORT     (unsigned int)(0x0000000000008000)
// Don't hit same species, explode on block.
// Player missiles as well as fireballs of various kinds.
#define MF_MISSILE      (unsigned int)(0x0000000000010000)
// Dropped by a demon, not level spawned.
// E.g. ammo clips dropped by dying former humans.
#define MF_DROPPED      (unsigned int)(0x0000000000020000)
// Use fuzzy draw (shadow demons or spectres),
//  temporary player invisibility powerup.
#define MF_SHADOW       (unsigned int)(0x0000000000040000)
// Flag: don't bleed when shot (use puff),
//  barrels and shootable furniture shall not bleed.
#define MF_NOBLOOD      (unsigned int)(0x0000000000080000)
// Don't stop moving halfway off a step,
//  that is, have dead bodies slide down all the way.
#define MF_CORPSE       (unsigned int)(0x0000000000100000)
// Floating to a height for a move, ???
//  don't auto float to target's height.
#define MF_INFLOAT      (unsigned int)(0x0000000000200000)

// On kill, count this enemy object
//  towards intermission kill total.
// Happy gathering.
#define MF_COUNTKILL    (unsigned int)(0x0000000000400000)

// On picking up, count this item object
//  towards intermission item total.
#define MF_COUNTITEM    (unsigned int)(0x0000000000800000)

// Special handling: skull in flight.
// Neither a cacodemon nor a missile.
#define MF_SKULLFLY     (unsigned int)(0x0000000001000000)

// Don't spawn this object
//  in death match mode (e.g. key cards).
#define MF_NOTDMATCH    (unsigned int)(0x0000000002000000)

// Player sprites in multiplayer modes are modified
//  using an internal color lookup table for re-indexing.
// If 0x4 0x8 or 0xc,
//  use a translation table for player colormaps
#define MF_TRANSLATION  (unsigned int)(0x000000000c000000)
#define MF_TRANSLATION1 (unsigned int)(0x0000000004000000)
#define MF_TRANSLATION2 (unsigned int)(0x0000000008000000)
// Hmm ???.
#define MF_TRANSSHIFT 26

//#define MF_UNUSED2      (unsigned int)(0x0000000010000000)
#define MF_DECORATION      (unsigned int)(0x0000000010000000) // next-hack.

#define MF_STATIC  (MF_DECORATION | MF_SPECIAL)				  // next-hack. Objects with this flags go in the static mobj zone, and occupy less RAM

#define MF_UNUSED3      (unsigned int)(0x0000000020000000)

// Translucent sprite?                                          // phares
#define MF_TRANSLUCENT  (unsigned int)(0x0000000040000000)

#define MF_FRIEND       (unsigned int)(0x0000000080000000)

// killough 9/15/98: Same, but internal flags, not intended for .deh
// (some degree of opaqueness is good, to avoid compatibility woes)

enum
{
    MIF_FALLING = 1      // Object is falling
};

// Map Object definition.
//
//
// killough 2/20/98:
//
// WARNING: Special steps must be taken in p_saveg.c if C pointers are added to
// this mobj_s struct, or else savegames will crash when loaded. See p_saveg.c.
// Do not add "struct mobj_s *fooptr" without adding code to p_saveg.c to
// convert the pointers to ordinals and back for savegames. This was the whole
// reason behind monsters going to sleep when loading savegames (the "target"
// pointer was simply nullified after loading, to prevent Doom from crashing),
// and the whole reason behind loadgames crashing on savegames of AV attacks.
//

// killough 9/8/98: changed some fields to shorts,
// for better memory usage (if only for cache).
/* cph 2006/08/28 - move Prev[XYZ] fields to the end of the struct. Add any
 * other new fields to the end, and make sure you don't break savegames! */

// 18/06/2021 next-hack. New Simplicity Studio 5.2 comes with a newer GNU
// toolchain version and it complains that member thinker might be unaligned as
// mobj_t and static_mobj_t are packed.
// We could pass simply the mobj address, and cast it to thinker_t*, but it is
// better to use unpacked structures at this point. Noticeably, there should
// not be any change in size because we carefully we reordered members to prevent
// paddings but to be sure we don't accidentally mess, we declare our expected
// size here, and an array that has null size if the size is correct, otherwise
// it will have a negative size, generating an error.
//
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
#define MOBJ_SIZE sizeof(mobj_t)
#define STATIC_MOBJ_SIZE sizeof(static_mobj_t)
#else
#define MOBJ_SIZE 92
#define STATIC_MOBJ_SIZE 44
#endif
//
typedef struct mobj_s
{
    // WARNING: the order of these variables is important! It must be kept the same as static_mobj
    // List: thinker links.
    thinker_t thinker;
    // killough 11/98: the lowest floor over all contacted Sectors.
    unsigned int flags;  // must be 32bit at least
    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t bnext_sptr;
    size_t bprev_sptr;
    size_t snext_sptr;
    size_t sprev_sptr;
#else
    unsigned short bnext_sptr;
    unsigned short bprev_sptr;
    // sector linked list. Note: short pointers.
    unsigned short snext_sptr;
    unsigned short sprev_sptr;
    // a linked list of sectors where this object appears
#endif
    // For movement checking.
    short tics:14;   // state tic counter
    unsigned short lastlook:2;
    uint8_t type;   // less than 255 mobjtypes.
    uint8_t sprite; // used to find patch_t and flip value
    //
    uint16_t frame; // might be ORed with FF_FULLBRIGHT (0x8000) but we do not need an int
    unsigned short state_idx :11; // there are less than 2048 states
    //short              movedir;        // 0-8

    //
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t subsector_sptr;
    size_t touching_sectorlist_sptr;
#else
    unsigned short subsector_sptr;
    unsigned short touching_sectorlist_sptr;
#endif
    //
    // Info for drawing: position.
    fixed_t x;
    fixed_t y;
    fixed_t z;
    //
    // The closest interval over all contacted Sectors.
    fixed_t ceilingz;
    fixed_t floorz;
    fixed_t dropoffz;

    //
    uint8_t height_s;  // we could reduce this too
    uint8_t radiusb;  // now it is a byte...

    int16_t health;   // max is 4000 but we need a negative value

    // AFTER THIS LINE, THE SUBSEQUENT PARAMETERS ARE MISSING IN THE STATIC MOBJ TYPE
    //More drawing info: to determine current sprite.
    angle_t angle;  // orientation
    //

    // Momentums, used to update position.
    fixed_t momx;
    fixed_t momy;
    fixed_t momz;
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t player_sptr;
    size_t lastenemy_sptr;
    size_t target_sptr;
    size_t tracer_sptr;
#else
    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    unsigned short player_sptr;
    // new field: last known enemy -- killough 2/15/98
    //struct mobj_s*      lastenemy;
    unsigned short lastenemy_sptr;
    //
    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    //struct mobj_s*      target;
    unsigned short target_sptr;
    // Thing being chased/attacked for tracers.
    //struct mobj_s*      tracer;
    unsigned short tracer_sptr;
#endif
    //
    //
    //    const state_t*      state;
    //
    // Movement direction, movement generation (zig-zagging).
    int movecount :12;      // when 0, select a new dir
    int strafecount :12;    // killough 9/8/98: monster strafing
    int movedir :5;        // 0-8

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    // short               reactiontime;
    uint8_t reactiontime;
    //
    // If >0, the current target will be chased no
    // matter what (even if shot by another object)
    uint8_t threshold;  // could be uint8_t
    //
    // killough 9/9/98: How long a monster pursues a target.
    uint8_t pursuecount;
    uint8_t dummy;

} mobj_t;
//
typedef struct static_mobj_s
{
    // WARNING: the order of these variables is important! It must be kept the same as static_mobj
    // List: thinker links.
    thinker_t thinker;
    // The closest interval over all contacted Sectors.
    // killough 11/98: the lowest floor over all contacted Sectors.
    unsigned int flags;  // must be 32bit at least
    // Interaction info, by BLOCKMAP.
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t bnext_sptr;
    size_t bprev_sptr;
    size_t snext_sptr;
    size_t sprev_sptr;
#else
    // Links in blocks (if needed).
    unsigned short bnext_sptr;
    unsigned short bprev_sptr;
    // sector linked list. Note: short pointers.
    unsigned short snext_sptr;
    unsigned short sprev_sptr;
    // a linked list of sectors where this object appears
#endif
    // For movement checking.
    short tics:14;   // state tic counter
    unsigned short lastlook:2;
    uint8_t type;   // less than 255 mobjtypes.
    // Actually we do not really need a sprite, it is used very seldom
    uint8_t sprite; // used to find patch_t and flip value
    //
    uint16_t frame; // might be ORed with FF_FULLBRIGHT (0x8000) but we do not need an int
    unsigned short state_idx :11; // there are less than 2048 states
    //short              movedir;        // 0-8

    //
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    size_t subsector_sptr;
    size_t touching_sectorlist_sptr;
#else
    unsigned short subsector_sptr;
    unsigned short touching_sectorlist_sptr;
#endif
    //
    // Info for drawing: position.
    fixed_t x;
    fixed_t y;
    fixed_t z;
}  static_mobj_t;

static inline fixed_t getMobjHeight(mobj_t *pmobj)
{
    if (!(pmobj->flags & MF_STATIC))
        return pmobj->height_s << FRACBITS;
    else
        return mobjinfo[pmobj->type].height;
}
static inline fixed_t getMobjRadius(mobj_t *pmobj)
{
    if (!(pmobj->flags & MF_STATIC))
        return pmobj->radiusb << FRACBITS;
    else
        return mobjinfo[pmobj->type].radius;

}
static inline const mobjinfo_t* getMobjInfo(mobj_t *pmobj)
{
    return &mobjinfo[pmobj->type];
}
static inline mobj_t* getLastEnemy(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->lastenemy_sptr);
}
static inline mobj_t* getTracer(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->tracer_sptr);
}
static inline mobj_t* getTarget(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->target_sptr);
}
static inline mobj_t* getSNext(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->snext_sptr);
}
static inline mobj_t* getSPrev(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->sprev_sptr);
}
static inline mobj_t* getBNext(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->bnext_sptr);
}
static inline mobj_t* getBPrev(mobj_t *pmobj)
{
    return (mobj_t*) getLongPtr(pmobj->bprev_sptr);
}

static inline struct player_s* getMobjPlayer(mobj_t *pmobj)
{
    return (struct player_s*) getLongPtr(pmobj->player_sptr);
}
static inline const state_t* getMobjState(mobj_t *pmobj)
{
    unsigned short index = pmobj->state_idx;
    if (index >= NUMSTATES || index == S_NULL) // TODO: is really needed this one?
        return NULL;
    return &states[index];
}
// External declarations (fomerly in p_local.h) -- killough 5/2/98

#define VIEWHEIGHT      (41*FRACUNIT)

#define GRAVITY         FRACUNIT
#define MAXMOVE         (30*FRACUNIT)

#define ONFLOORZ        INT_MIN
#define ONCEILINGZ      INT_MAX

// Time interval for item respawning.
#define ITEMQUESIZE     32

#define FLOATSPEED      (FRACUNIT*4)
#define STOPSPEED       (FRACUNIT/16)

// killough 11/98:
// For torque simulation:

#define OVERDRIVE 6
#define MAXGEAR (OVERDRIVE+16)

// killough 11/98:
// Whether an object is "sentient" or not. Used for environmental influences.
#define sentient(mobj) ((mobj)->health > 0 && (mobj)->info->seestate)

void P_RespawnSpecials(void);
mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
static_mobj_t* P_SpawnStaticMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type); // 2021-03-13 next-hack
void P_RemoveMobj(mobj_t *th);
void P_RemoveStaticMobj(static_mobj_t *mobj);  // 2021-03-13 next-hack

boolean P_SetMobjState(mobj_t *mobj, statenum_t state);

void P_MobjThinker(mobj_t *mobj);
void P_MobjBrainlessThinker(mobj_t *mobj);

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage);
mobj_t* P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
boolean P_IsDoomnumAllowed(int doomnum);
void P_SpawnMapThing(const mapthing_t *mthing);
void P_SpawnPlayer(int n, const mapthing_t *mthing);
void P_CheckMissileSpawn(mobj_t*);  // killough 8/2/98
void P_ExplodeMissile(mobj_t*);    // killough
#endif

