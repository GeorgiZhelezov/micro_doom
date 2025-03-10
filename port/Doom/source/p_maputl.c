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
 *      Movement/collision utility functions,
 *      as used by function in p_map.c.
 *      BLOCKMAP Iterator functions,
 *      and some PIT_* functions to use for iteration.
 *
 *      next-hack:
 *      modified code to work with short pointers.
 *      also reverted pointer-to-pointer scheme back to original because
 *      it is simpler to handle with short pointers.
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_bbox.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"

#include "global_data.h"
intercept_t intercepts[MAXINTERCEPTS];
boolean interceptIsALine[MAXINTERCEPTS];
intercept_t *intercept_p;
//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t CONSTFUNC P_AproxDistance(fixed_t dx, fixed_t dy)
{
    dx = D_abs(dx);
    dy = D_abs(dy);
    if (dx < dy)
        return dx + dy - (dx >> 1);
    return dx + dy - (dy >> 1);
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
// killough 5/3/98: reformatted, cleaned up

int PUREFUNC P_PointOnLineSide(fixed_t x, fixed_t y, const line_t *line)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    line_t temp_line;
    user_flash_read_game_resource(&temp_line, sizeof(temp_line), (uint32_t)line);
    debugi("%s x:%d y:%d line at %08x line.dx:%d line.dy:%d line.v1.x:%d line.v1.y:%d\r\n",
           __func__,
           x,
           y,
           (uint32_t)line,
           temp_line.dx,
           temp_line.dy,
           temp_line.v1.x,
           temp_line.v1.y);
    line = &temp_line;
#else
    debugi("%s x:%d y:%d line at %08x line.dx:%d line.dy:%d line.v1.x:%d line.v1.y:%d\r\n",
           __func__,
           x,
           y,
           (uint32_t)line,
           line->dx,
           line->dy,
           line->v1.x,
           line->v1.y);
#endif
    return !line->dx ? x <= line->v1.x ? line->dy > 0 : line->dy < 0
           :
           !line->dy ? y <= line->v1.y ? line->dx < 0 : line->dx > 0:
           FixedMul(y - line->v1.y, line->dx >> FRACBITS) >= FixedMul(line->dy >> FRACBITS, x - line->v1.x);
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
// killough 5/3/98: reformatted, cleaned up

int PUREFUNC P_BoxOnLineSide(const fixed_t *tmbox, const line_t *ld)
{
    int p;
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    line_t temp_line;
    user_flash_read_game_resource(&temp_line, sizeof(temp_line), (uint32_t)ld);
    debugi("%s line at %08x line.slopetype:%d\r\n", __func__, (uint32_t)ld, temp_line.slopetype);
    switch (temp_line.slopetype)
    {

        default: // shut up compiler warnings -- killough
        case ST_HORIZONTAL:
            return (tmbox[BOXBOTTOM] > temp_line.v1.y) == (p = tmbox[BOXTOP] > temp_line.v1.y) ? p ^ (temp_line.dx < 0) : -1;
        case ST_VERTICAL:
            return (tmbox[BOXLEFT] < temp_line.v1.x) == (p = tmbox[BOXRIGHT] < temp_line.v1.x) ? p ^ (temp_line.dy < 0) : -1;
        case ST_POSITIVE:
            return P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) == (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;
        case ST_NEGATIVE:
            return (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) == (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
#else
    debugi("%s line at %08x line.slopetype:%d\r\n", __func__, (uint32_t)ld, ld->slopetype);
    switch (ld->slopetype)
    {

        default: // shut up compiler warnings -- killough
        case ST_HORIZONTAL:
            return (tmbox[BOXBOTTOM] > ld->v1.y) == (p = tmbox[BOXTOP] > ld->v1.y) ? p ^ (ld->dx < 0) : -1;
        case ST_VERTICAL:
            return (tmbox[BOXLEFT] < ld->v1.x) == (p = tmbox[BOXRIGHT] < ld->v1.x) ? p ^ (ld->dy < 0) : -1;
        case ST_POSITIVE:
            return P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) == (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;
        case ST_NEGATIVE:
            return (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) == (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
#endif
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
// killough 5/3/98: reformatted, cleaned up

static int PUREFUNC P_PointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line)
{
    return !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0
           :
           !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0
           :
           (line->dy ^ line->dx ^ (x -= line->x) ^ (y -= line->y)) < 0 ? (line->dy ^ x) < 0 : FixedMul(y >> 8, line->dx >> 8) >= FixedMul(line->dy >> 8, x >> 8);
}

//
// P_MakeDivline
//

static void P_MakeDivline(const line_t *li, divline_t *dl)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    line_t temp_line;
    user_flash_read_game_resource(&temp_line, sizeof(temp_line), (uint32_t)li);
    li = &temp_line;
#endif
    dl->x = li->v1.x;
    dl->y = li->v1.y;
    dl->dx = li->dx;
    dl->dy = li->dy;

    debugi("%s x:%d y:%d dx:%d dy:%d\r\n", __func__, dl->x, dl->y, dl->dx, dl->dy);
}

//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
//

/* cph - this is killough's 4/19/98 version of P_InterceptVector and
 *  P_InterceptVector2 (which were interchangeable). We still use this
 *  in compatibility mode. */
fixed_t PUREFUNC P_InterceptVector2(const divline_t *v2, const divline_t *v1)
{
    fixed_t den;
    return ((den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy))) ? FixedDiv(FixedMul((v1->x - v2->x) >> 8, v1->dy) + FixedMul((v2->y - v1->y) >> 8, v1->dx), den) : 0;
}

//
// P_LineOpening
// Sets opentop and openbottom to the window
// through a two sided line.
// OPTIMIZE: keep this precalculated
//

void P_LineOpening(const line_t *linedef)
{
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    line_t temp_line;
    user_flash_read_game_resource(&temp_line, sizeof(temp_line), (uint32_t)linedef);
    if (temp_line.sidenum[1] == NO_INDEX)      // single sided line
    {
        _g->openrange = 0;
        return;
    }
    side_t temp_side0;
    user_flash_read_game_resource(&temp_side0, sizeof(temp_side0), (uint32_t)(_g->sides + temp_line.sidenum[0]));
    side_t temp_side1;
    user_flash_read_game_resource(&temp_side1, sizeof(temp_side1), (uint32_t)(_g->sides + temp_line.sidenum[1]));

    _g->openfrontsector = LN_FRONTSECTOR_ALT(temp_line, temp_side0);
    _g->openbacksector = LN_BACKSECTOR_ALT(&temp_line, temp_side1);

    debugi("%s openfrontsec.ceilh:%d openfrontsec.floorh:%d\r\n", __func__, _g->openfrontsector->ceilingheight, _g->openfrontsector->floorheight);
#else
    if (linedef->sidenum[1] == NO_INDEX)      // single sided line
    {
        _g->openrange = 0;
        return;
    }

    _g->openfrontsector = LN_FRONTSECTOR(linedef);
    _g->openbacksector = LN_BACKSECTOR(linedef);
    debugi("%s openfrontsec.ceilh:%d openfrontsec.floorh:%d\r\n", __func__, _g->openfrontsector->ceilingheight, _g->openfrontsector->floorheight);
#endif

    if (_g->openfrontsector->ceilingheight < _g->openbacksector->ceilingheight)
        _g->opentop = _g->openfrontsector->ceilingheight;
    else
        _g->opentop = _g->openbacksector->ceilingheight;

    if (_g->openfrontsector->floorheight > _g->openbacksector->floorheight)
    {
        _g->openbottom = _g->openfrontsector->floorheight;
        _g->lowfloor = _g->openbacksector->floorheight;
    }
    else
    {
        _g->openbottom = _g->openbacksector->floorheight;
        _g->lowfloor = _g->openfrontsector->floorheight;
    }
    _g->openrange = _g->opentop - _g->openbottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//

void P_UnsetThingPosition(mobj_t *thing)
{
    if (!(thing->flags & MF_NOSECTOR ))
    {
        // invisible things don't need to be in sector list
        // unlink from subsector
        if (thing->snext_sptr)
            getSNext(thing)->sprev_sptr = thing->sprev_sptr;
        if (thing->sprev_sptr)
            getSPrev(thing)->snext_sptr = thing->snext_sptr;
        else
        {
            //		    getMobjSubsector(thing)->sector->thinglist = getLongPtr(thing->snext_sptr);
            getMobjSubsector(thing)->sector->thinglist_sptr = thing->snext_sptr;
        }
        // phares 3/14/98
        //
        // Save the sector list pointed to by touching_sectorlist.
        // In P_SetThingPosition, we'll keep any nodes that represent
        // sectors the Thing still touches. We'll add new ones then, and
        // delete any nodes for sectors the Thing has vacated. Then we'll
        // put it back into touching_sectorlist. It's done this way to
        // avoid a lot of deleting/creating for nodes, when most of the
        // time you just get back what you deleted anyway.
        //
        // If this Thing is being removed entirely, then the calling
        // routine will clear out the nodes in sector_list.

        _g->sector_list = getTouchingSectorList(thing);
        thing->touching_sectorlist_sptr = 0;
    }

    if (!(thing->flags & MF_NOBLOCKMAP ))
    {
        /* inert things don't need to be in blockmap
         */
        int blockx;
        int blocky;
        if (thing->bnext_sptr)
            getBNext(thing)->bprev_sptr = thing->bprev_sptr;

        if (thing->bprev_sptr)
            getBPrev(thing)->bnext_sptr = thing->bnext_sptr;
        else
        {
            blockx = (thing->x - _g->bmaporgx) >> MAPBLOCKSHIFT;
            blocky = (thing->y - _g->bmaporgy) >> MAPBLOCKSHIFT;

            if (blockx >= 0 && blockx < _g->bmapwidth && blocky >= 0 && blocky < _g->bmapheight)
            {
                _g->blocklinks_sptrs[blocky * _g->bmapwidth + blockx] = thing->bnext_sptr;
            }
        }
    }
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
// killough 5/3/98: reformatted, cleaned up
void P_SetThingPosition(mobj_t *thing)
{                                                      // link into subsector
    debugi("%s setting thing pos x:%d y:%d flags:%d\r\n", __func__, thing->x, thing->y, thing->flags);
    subsector_t *ss = setMobjSubesctor(thing, R_PointInSubsector(thing->x, thing->y));
    if (!(thing->flags & MF_NOSECTOR ))
    {
        // invisible things don't go into the sector links
        sector_t *sec = ss->sector;

        thing->sprev_sptr = 0;
        thing->snext_sptr = sec->thinglist_sptr;
        if (sec->thinglist_sptr)
            getSectorThingList(sec)->sprev_sptr = getShortPtr(thing);
        sec->thinglist_sptr = getShortPtr(thing);

        // phares 3/16/98
        //
        // If sector_list isn't NULL, it has a collection of sector
        // nodes that were just removed from this Thing.

        // Collect the sectors the object will live in by looking at
        // the existing sector_list and adding new nodes and deleting
        // obsolete ones.

        // When a node is deleted, its sector links (the links starting
        // at sector_t->touching_thinglist) are broken. When a node is
        // added, new sector links are created.

        P_CreateSecNodeList(thing, thing->x, thing->y);
        thing->touching_sectorlist_sptr = getShortPtr(_g->sector_list); // Attach to Thing's mobj_t
        _g->sector_list = NULL; // clear for next time
    }

    // link into blockmap
    if (!(thing->flags & MF_NOBLOCKMAP ))
    {
        // inert things don't need to be in blockmap
        int blockx = (thing->x - _g->bmaporgx) >> MAPBLOCKSHIFT;
        int blocky = (thing->y - _g->bmaporgy) >> MAPBLOCKSHIFT;
        if (blockx >= 0 && blockx < _g->bmapwidth && blocky >= 0 && blocky < _g->bmapheight)
        {
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
            size_t *link = &_g->blocklinks_sptrs[blocky * _g->bmapwidth + blockx];
#else
            unsigned short *link = &_g->blocklinks_sptrs[blocky * _g->bmapwidth + blockx];
#endif
            thing->bprev_sptr = 0;
            thing->bnext_sptr = *link;
            if (*link)
                ((mobj_t*) getLongPtr(*link))->bprev_sptr = getShortPtr(thing);
            *link = getShortPtr(thing);
        }
        else
        {
            // thing is off the map
            thing->bnext_sptr = thing->bprev_sptr = 0;
        }
    }
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
// killough 5/3/98: reformatted, cleaned up

boolean P_BlockLinesIterator(int x, int y, boolean func(const line_t*))
{

    if (x < 0 || y < 0 || x >= _g->bmapwidth || y >= _g->bmapheight)
        return true;

#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    short temp;
    user_flash_read_game_resource(&temp, sizeof(temp), (uint32_t)(_g->blockmap + (y * _g->bmapwidth + x)));
    int offset = temp;
    debugi("%s: offset read at %08x offset:%d\r\n", __func__, (uint32_t)(_g->blockmap + (y * _g->bmapwidth + x)), offset);
#else
    const int offset = _g->blockmap[y * _g->bmapwidth + x];
    debugi("%s: offset read at %08x offset:%d\r\n", __func__, (uint32_t)&_g->blockmap[y * _g->bmapwidth + x], offset);
#endif
    const short *list = _g->blockmaplump + offset; // original was reading         // phares

    // delmiting 0 as linedef 0     // phares

    // killough 1/31/98: for compatibility we need to use the old method.
    // Most demos go out of sync, and maybe other problems happen, if we
    // don't consider linedef 0. For safety this should be qualified.
    if (!demo_compatibility) // killough 2/22/98: demo_compatibility check
        list++;     // skip 0 starting delimiter                      // phares

    const uint16_t vcount = _g->validcount;

#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    for (; ; list++)                                   // phares
    {
        short temp_list;
        user_flash_read_game_resource(&temp_list, sizeof(temp_list), (uint32_t)list);   
        if (temp_list == -1) { break; }

        const int lineno = temp_list;

        linedata_t *lt = &_g->linedata[lineno];

        debugi("%s: list iteration at %08x lineno:%d lt.validcount:%d vcount:%d x:%d y:%d\r\n",
               __func__,
               (uint32_t)list,
               lineno,
               lt->validcount,
               vcount,
               x,
               y);
        if (lt->validcount == vcount)
            continue;       // line has already been checked

        lt->validcount = vcount;

        const line_t *ld = &_g->lines[lineno];

        if (!func(ld))
            return false;
    }
#else
    for (; *list != -1; list++)                                   // phares
    {
        const int lineno = *list;

        linedata_t *lt = &_g->linedata[lineno];

        debugi("%s: list iteration at %08x lineno:%d lt.validcount:%d vcount:%d x:%d y:%d\r\n",
               __func__,
               (uint32_t)list,
               lineno,
               lt->validcount,
               vcount,
               x,
               y);
        if (lt->validcount == vcount)
            continue;       // line has already been checked

        lt->validcount = vcount;

        const line_t *ld = &_g->lines[lineno];

        if (!func(ld))
            return false;
    }
#endif
    return true;  // everything was checked
}

//
// P_BlockThingsIterator
//
// killough 5/3/98: reformatted, cleaned up

boolean P_BlockThingsIterator(int x, int y, boolean func(mobj_t*))
{
    mobj_t *mobj;
    if (!(x < 0 || y < 0 || x >= _g->bmapwidth || y >= _g->bmapheight))
        for (mobj = getLongPtr(_g->blocklinks_sptrs[y * _g->bmapwidth + x]);
                mobj; mobj = getBNext(mobj))
        {
            if (!func(mobj))
                return false;
        }
    return true;
}

//
// INTERCEPT ROUTINES
//

// Check for limit and double size if necessary -- killough
static boolean check_intercept(void)
{
    size_t offset = intercept_p - intercepts;
    if (offset >= MAXINTERCEPTS)
    {
        printf ("Too many intercepts, blocking");
        while(1);
    }
    return (offset < MAXINTERCEPTS);
}

// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
//
// killough 5/3/98: reformatted, cleaned up

boolean PIT_AddLineIntercepts(const line_t *ld)
{
    int s1;
    int s2;
    fixed_t frac;
    divline_t dl;

    // avoid precision problems with two routines
    if (_g->trace.dx > FRACUNIT * 16 || _g->trace.dy > FRACUNIT * 16 || _g->trace.dx < -FRACUNIT * 16 || _g->trace.dy < -FRACUNIT * 16)
    {
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
        line_t temp_line;
        user_flash_read_game_resource(&temp_line, sizeof(temp_line), (uint32_t)ld);
        s1 = P_PointOnDivlineSide(temp_line.v1.x, temp_line.v1.y, &_g->trace);
        s2 = P_PointOnDivlineSide(temp_line.v2.x, temp_line.v2.y, &_g->trace);
        debugi("%s line at %08x s1:%d s2:%d\r\n", __func__, (uint32_t)ld, s1, s2);
#else
        s1 = P_PointOnDivlineSide(ld->v1.x, ld->v1.y, &_g->trace);
        s2 = P_PointOnDivlineSide(ld->v2.x, ld->v2.y, &_g->trace);
        debugi("%s line at %08x s1:%d s2:%d\r\n", __func__, (uint32_t)ld, s1, s2);
#endif
    }
    else
    {
        s1 = P_PointOnLineSide(_g->trace.x, _g->trace.y, ld);
        s2 = P_PointOnLineSide(_g->trace.x + _g->trace.dx, _g->trace.y + _g->trace.dy, ld);
    }

    if (s1 == s2)
        return true;        // line isn't crossed

    // hit the line
    P_MakeDivline(ld, &dl);
    frac = P_InterceptVector2(&_g->trace, &dl);

    if (frac < 0)
        return true;        // behind source

    if (!check_intercept())
        return false;

    int index = (int) (intercept_p - intercepts);
        checkInterceptIndex(index);
    interceptIsALine[index] = true;
    //

    intercept_p->frac = frac;
    //intercept_p->isaline = true;
    intercept_p->d.line = ld;
    intercept_p++;

    return true;  // continue
}

//
// PIT_AddThingIntercepts
//
// killough 5/3/98: reformatted, cleaned up

boolean PIT_AddThingIntercepts(mobj_t *thing)
{
    fixed_t x1, y1;
    fixed_t x2, y2;
    int s1, s2;
    divline_t dl;
    fixed_t frac;

    // check a corner to corner crossection for hit
    if ((_g->trace.dx ^ _g->trace.dy) > 0)
    {
        x1 = thing->x - getMobjRadius(thing);
        y1 = thing->y + getMobjRadius(thing);
        x2 = thing->x + getMobjRadius(thing);
        y2 = thing->y - getMobjRadius(thing);
    }
    else
    {
        x1 = thing->x - getMobjRadius(thing);
        y1 = thing->y - getMobjRadius(thing);
        x2 = thing->x + getMobjRadius(thing);
        y2 = thing->y + getMobjRadius(thing);
    }

    s1 = P_PointOnDivlineSide(x1, y1, &_g->trace);
    s2 = P_PointOnDivlineSide(x2, y2, &_g->trace);

    if (s1 == s2)
        return true;                // line isn't crossed

    dl.x = x1;
    dl.y = y1;
    dl.dx = x2 - x1;
    dl.dy = y2 - y1;

    frac = P_InterceptVector2(&_g->trace, &dl);

    if (frac < 0)
        return true;                // behind source

    if (!check_intercept())
        return false;
    int index = (int) (intercept_p - intercepts);
    checkInterceptIndex(index);
    interceptIsALine[index] = false;

    intercept_p->frac = frac;
//  intercept_p->isaline = false;
    intercept_p->d.thing = thing;
    intercept_p++;

    return true;          // keep going
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
// killough 5/3/98: reformatted, cleaned up

boolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac)
{
    intercept_t *in = NULL;
    int count = intercept_p - intercepts;
    while (count--)
    {
        fixed_t dist = INT_MAX;
        intercept_t *scan;
        for (scan = intercepts; scan < intercept_p; scan++)
            if (scan->frac < dist)
                dist = (in = scan)->frac;
        if (dist > maxfrac)
            return true;    // checked everything in range
        if (!func(in))
            return false;           // don't bother going farther
        in->frac = INT_MAX;
    }
    return true;                  // everything was traversed
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
// killough 5/3/98: reformatted, cleaned up
#if 1
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, boolean trav(intercept_t*))
{
    fixed_t xt1, yt1;
    fixed_t xt2, yt2;
    fixed_t xstep, ystep;
    fixed_t partial;
    fixed_t xintercept, yintercept;
    int mapx, mapy;
    int mapxstep, mapystep;
    int count;

    _g->validcount++;
    intercept_p = intercepts;

    if (!((x1 - _g->bmaporgx) & (MAPBLOCKSIZE - 1)))
        x1 += FRACUNIT;     // don't side exactly on a line

    if (!((y1 - _g->bmaporgy) & (MAPBLOCKSIZE - 1)))
        y1 += FRACUNIT;     // don't side exactly on a line

    _g->trace.x = x1;
    _g->trace.y = y1;
    _g->trace.dx = x2 - x1;
    _g->trace.dy = y2 - y1;

    x1 -= _g->bmaporgx;
    y1 -= _g->bmaporgy;
    xt1 = x1 >> MAPBLOCKSHIFT;
    yt1 = y1 >> MAPBLOCKSHIFT;

    x2 -= _g->bmaporgx;
    y2 -= _g->bmaporgy;
    xt2 = x2 >> MAPBLOCKSHIFT;
    yt2 = y2 >> MAPBLOCKSHIFT;

    if (xt2 > xt1)
    {
        mapxstep = 1;
        partial = FRACUNIT - ((x1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        ystep = FixedDiv(y2 - y1, D_abs(x2 - x1));
    }
    else if (xt2 < xt1)
    {
        mapxstep = -1;
        partial = (x1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        ystep = FixedDiv(y2 - y1, D_abs(x2 - x1));
    }
    else
    {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256 * FRACUNIT;
    }

    yintercept = (y1 >> MAPBTOFRAC) + FixedMul(partial, ystep);

    if (yt2 > yt1)
    {
        mapystep = 1;
        partial = FRACUNIT - ((y1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        xstep = FixedDiv(x2 - x1, D_abs(y2 - y1));
    }
    else if (yt2 < yt1)
    {
        mapystep = -1;
        partial = (y1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        xstep = FixedDiv(x2 - x1, D_abs(y2 - y1));
    }
    else
    {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256 * FRACUNIT;
    }

    xintercept = (x1 >> MAPBTOFRAC) + FixedMul(partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.

    mapx = xt1;
    mapy = yt1;

    for (count = 0; count < 64; count++)
    {
        if (flags & PT_ADDLINES)
            if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
                return false; // early out

        if (flags & PT_ADDTHINGS)
            if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
                return false; // early out

        if (mapx == xt2 && mapy == yt2)
            break;

        if ((yintercept >> FRACBITS) == mapy)
        {
            yintercept += ystep;
            mapx += mapxstep;
        }
        else if ((xintercept >> FRACBITS) == mapx)
        {
            xintercept += xstep;
            mapy += mapystep;
        }
    }

    // go through the sorted list
    return P_TraverseIntercepts(trav, FRACUNIT);
}
#elif 0
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, boolean trav(intercept_t*))
{
    fixed_t xt1, yt1;
    fixed_t xt2, yt2;
    fixed_t xstep, ystep;
    fixed_t partial;
    fixed_t xintercept, yintercept;
    int mapx, mapy;
  int     mapx1, mapy1;
    int mapxstep, mapystep;
    int count;

    _g->validcount++;
    intercept_p = intercepts;

    if (!((x1 - _g->bmaporgx) & (MAPBLOCKSIZE - 1)))
        x1 += FRACUNIT;     // don't side exactly on a line

    if (!((y1 - _g->bmaporgy) & (MAPBLOCKSIZE - 1)))
        y1 += FRACUNIT;     // don't side exactly on a line

    _g->trace.x = x1;
    _g->trace.y = y1;
    _g->trace.dx = x2 - x1;
    _g->trace.dy = y2 - y1;

  if (demo_compatibility)
  {
    int_64_t _x1, _x2, _y1, _y2;

    _x1 = (int_64_t)x1 - _g->bmaporgx;
    _y1 = (int_64_t)y1 - _g->bmaporgy;
    xt1 = (int)(_x1>>MAPBLOCKSHIFT);
    yt1 = (int)(_y1>>MAPBLOCKSHIFT);

    mapx1 = (int)(_x1>>MAPBTOFRAC);
    mapy1 = (int)(_y1>>MAPBTOFRAC);

    _x2 = (int_64_t)x2 - _g->bmaporgx;
    _y2 = (int_64_t)y2 - _g->bmaporgy;
    xt2 = (int)(_x2>>MAPBLOCKSHIFT);
    yt2 = (int)(_y2>>MAPBLOCKSHIFT);

    x1 -= _g->bmaporgx;
    y1 -= _g->bmaporgy;
    x2 -= _g->bmaporgx;
    y2 -= _g->bmaporgy;
  }
  else
  {
    x1 -= _g->bmaporgx;
    y1 -= _g->bmaporgy;
    xt1 = x1>>MAPBLOCKSHIFT;
    yt1 = y1>>MAPBLOCKSHIFT;

    mapx1 = x1>>MAPBTOFRAC;
    mapy1 = y1>>MAPBTOFRAC;

    x2 -= _g->bmaporgx;
    y2 -= _g->bmaporgy;
    xt2 = x2>>MAPBLOCKSHIFT;
    yt2 = y2>>MAPBLOCKSHIFT;
  }
    if (xt2 > xt1)
    {
        mapxstep = 1;
      partial = FRACUNIT - (mapx1&(FRACUNIT-1));
      ystep = FixedDiv (y2-y1,D_abs(x2-x1));
    }
  else
    if (xt2 < xt1)
    {
        mapxstep = -1;
        partial = mapx1&(FRACUNIT-1);
        ystep = FixedDiv (y2-y1,D_abs(x2-x1));
    }
    else
    {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256 * FRACUNIT;
    }

  yintercept = mapy1 + FixedMul(partial, ystep);

    if (yt2 > yt1)
    {
        mapystep = 1;
      partial = FRACUNIT - (mapy1&(FRACUNIT-1));
      xstep = FixedDiv (x2-x1,D_abs(y2-y1));
    }
    else if (yt2 < yt1)
    {
        mapystep = -1;
        partial = mapy1&(FRACUNIT-1);
        xstep = FixedDiv (x2-x1,D_abs(y2-y1));
    }
    else
    {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256 * FRACUNIT;
    }

  xintercept = mapx1 + FixedMul(partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.

    mapx = xt1;
    mapy = yt1;

    for (count = 0; count < 64; count++)
    {
        if (flags & PT_ADDLINES)
            if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
                return false; // early out

        if (flags & PT_ADDTHINGS)
            if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
                return false; // early out

        if (mapx == xt2 && mapy == yt2)
            break;

        if ((yintercept >> FRACBITS) == mapy)
        {
            yintercept += ystep;
            mapx += mapxstep;
        }
        else if ((xintercept >> FRACBITS) == mapx)
        {
            xintercept += xstep;
            mapy += mapystep;
        }
    }

    // go through the sorted list
    return P_TraverseIntercepts(trav, FRACUNIT);
}
#else
boolean
P_PathTraverse
( fixed_t		x1,
  fixed_t		y1,
  fixed_t		x2,
  fixed_t		y2,
  int			flags,
  boolean (*trav) (intercept_t *))
{
    fixed_t	xt1;
    fixed_t	yt1;
    fixed_t	xt2;
    fixed_t	yt2;
    
    fixed_t	xstep;
    fixed_t	ystep;
    
    fixed_t	partial;
    
    fixed_t	xintercept;
    fixed_t	yintercept;
    
    int		mapx;
    int		mapy;
    
    int		mapxstep;
    int		mapystep;

    int		count;
		
    //earlyout = (flags & PT_EARLYOUT) != 0;
		
    _g->validcount++;
    intercept_p = intercepts;
	
    if ( ((x1-_g->bmaporgx)&(MAPBLOCKSIZE-1)) == 0)
	x1 += FRACUNIT;	// don't side exactly on a line
    
    if ( ((y1-_g->bmaporgy)&(MAPBLOCKSIZE-1)) == 0)
	y1 += FRACUNIT;	// don't side exactly on a line

    _g->trace.x = x1;
    _g->trace.y = y1;
    _g->trace.dx = x2 - x1;
    _g->trace.dy = y2 - y1;

    x1 -= _g->bmaporgx;
    y1 -=_g-> bmaporgy;
    xt1 = x1>>MAPBLOCKSHIFT;
    yt1 = y1>>MAPBLOCKSHIFT;

    x2 -= _g->bmaporgx;
    y2 -= _g->bmaporgy;
    xt2 = x2>>MAPBLOCKSHIFT;
    yt2 = y2>>MAPBLOCKSHIFT;

    if (xt2 > xt1)
    {
	mapxstep = 1;
	partial = FRACUNIT - ((x1>>MAPBTOFRAC)&(FRACUNIT-1));
	ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
    else if (xt2 < xt1)
    {
	mapxstep = -1;
	partial = (x1>>MAPBTOFRAC)&(FRACUNIT-1);
	ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
    else
    {
	mapxstep = 0;
	partial = FRACUNIT;
	ystep = 256*FRACUNIT;
    }	

    yintercept = (y1>>MAPBTOFRAC) + FixedMul (partial, ystep);

	
    if (yt2 > yt1)
    {
	mapystep = 1;
	partial = FRACUNIT - ((y1>>MAPBTOFRAC)&(FRACUNIT-1));
	xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
    else if (yt2 < yt1)
    {
	mapystep = -1;
	partial = (y1>>MAPBTOFRAC)&(FRACUNIT-1);
	xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
    else
    {
	mapystep = 0;
	partial = FRACUNIT;
	xstep = 256*FRACUNIT;
    }	
    xintercept = (x1>>MAPBTOFRAC) + FixedMul (partial, xstep);
    
    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.
    mapx = xt1;
    mapy = yt1;
	
    for (count = 0 ; count < 64 ; count++)
    {
	if (flags & PT_ADDLINES)
	{
	    if (!P_BlockLinesIterator (mapx, mapy,PIT_AddLineIntercepts))
		return false;	// early out
	}
	
	if (flags & PT_ADDTHINGS)
	{
	    if (!P_BlockThingsIterator (mapx, mapy,PIT_AddThingIntercepts))
		return false;	// early out
	}
		
	if (mapx == xt2
	    && mapy == yt2)
	{
	    break;
	}
	
	if ( (yintercept >> FRACBITS) == mapy)
	{
	    yintercept += ystep;
	    mapx += mapxstep;
	}
	else if ( (xintercept >> FRACBITS) == mapx)
	{
	    xintercept += xstep;
	    mapy += mapystep;
	}
		
    }
    // go through the sorted list
    return P_TraverseIntercepts ( trav, FRACUNIT );
}

#endif