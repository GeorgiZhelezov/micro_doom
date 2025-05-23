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
 *      The status bar widget code.
 *
 * next-hack: modified status bar update code and added support for digit stored
 * in external flash.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_main.h"
#include "lprintf.h"
#include "global_data.h"

#include "i_spi_support.h"

#include "st_gfx.h"
//
// STlib_init()
//
void STlib_init(void)
{
    // cph - no longer hold STMINUS pointer
}

//
// STlib_initNum()
//
// Initializes an st_number_t widget
//
// Passed the widget, its position, the patches for the digits, a pointer
// to the value displayed, a pointer to the on/off control, and the width
// Returns nothing
//
void STlib_initNum(st_number_t *n, int x, int y, const patch_t **pl, short *num, boolean *on, int width)
{
    n->x = x;
    n->y = y;
    n->oldnum = 0;
    n->width = width;
    n->num = num;
    n->on = on;
    n->p = pl;
    // debugi("%s n.x:%d n.y:%d n.w:%d n.num:%d n.on:%d\r\n", __func__, n->x, n->y, n->width, *n->num, *n->on);
}

/*
 * STlib_drawNum()
 *
 * A fairly efficient way to draw a number based on differences from the
 * old number.
 *
 * Passed a st_number_t widget, a color range for output, and a flag
 * indicating whether refresh is needed.
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - const pointer to colour trans table, made function static
 */
static void STlib_drawNum(st_number_t *n, int cm, boolean refresh)
{

    int numdigits = n->width;
    int num = *n->num;

    int w;
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
    uint32_t temp_patch0_addr;
    user_flash_read_game_resource(&temp_patch0_addr, sizeof(temp_patch0_addr), (uint32_t)&n->p[0]);
    patch_t temp_patch0;
    user_flash_read_game_resource(&temp_patch0, sizeof(temp_patch0), temp_patch0_addr);
    uint32_t temp_width_addr = (uint32_t)(&((patch_t*)temp_patch0_addr)->width);
    // debugi("%s patch0 is at %08x width:%d width at %08x\r\n",
    //        __func__,
    //        temp_patch0_addr,
    //        temp_patch0.width,
    //        temp_width_addr);
    if (isOnExternalFlash(temp_width_addr))
    {
        spiFlashSetAddress((uint32_t) temp_width_addr);
        w = spiFlashGetShort();
    }
    else
        w = temp_patch0.width;
#else
    // debugi("%s patch0 is at %08x width:%d width at %08x\r\n",
        //    __func__,
        //    (uint32_t)n->p[0],
        //    n->p[0]->width,
        //    (uint32_t)&n->p[0]->width);
    if (isOnExternalFlash(&n->p[0]->width))
    {
        spiFlashSetAddress((uint32_t) &n->p[0]->width);
        w = spiFlashGetShort();
    }
    else
        w = n->p[0]->width;
#endif
    int x = n->x;

    int neg;

    // CPhipps - compact some code, use num instead of *n->num
    if ((neg = (n->oldnum = num) < 0))
    {
        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;

        num = -num;
    }

    // clear the area
    x = n->x - numdigits * w;

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    //jff 2/16/98 add color translation to digit output
    // in the special case of 0, you draw 0
    if (!num)
    {
        // CPhipps - patch drawing updated, reformatted
        // debugi("%s num:%d\r\n", __func__, num);
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
        V_DrawPatchNoScale(x - w, n->y, (const patch_t*)temp_patch0_addr);
#else
        V_DrawPatchNoScale(x - w, n->y, n->p[0]);
#endif
    }

    // draw the new number
    //jff 2/16/98 add color translation to digit output
    while (num && numdigits--)
    {
        // CPhipps - patch drawing updated, reformatted
        x -= w;
        // debugi("%s num:%d numdigits:%d\r\n", __func__, num, numdigits);
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
        uint32_t temp_patch_addr;
        user_flash_read_game_resource(&temp_patch_addr, sizeof(temp_patch_addr), (uint32_t)&n->p[num % 10]);

        // debugi("%s variable patch at %08x\r\n", __func__, temp_patch_addr);
        V_DrawPatchNoScale(x, n->y, (const patch_t*)temp_patch_addr);
#else
        // debugi("%s variable patch at %08x\r\n", __func__, (uint32_t)n->p[num%10]);
        V_DrawPatchNoScale(x, n->y, n->p[num % 10]);
#endif
        num /= 10;
    }
}

/*
 * STlib_updateNum()
 *
 * Draws a number conditionally based on the widget's enable
 *
 * Passed a number widget, the output color range, and a refresh flag
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - make that pointer const
 */
void STlib_updateNum(st_number_t *n, int cm, boolean refresh)
{
    if (*n->on)
    {
        // debugi("%s n.num:%d n.oldnum:%d n.width:%d n.x:%d n.y:%d cm:%d\r\n", __func__, *n->num, n->oldnum, n->width, n->x, n->y, cm);
        STlib_drawNum(n, cm, refresh);
    }
}

//
// STlib_initPercent()
//
// Initialize a st_percent_t number with percent sign widget
//
// Passed a st_percent_t widget, the position, the digit patches, a pointer
// to the number to display, a pointer to the enable flag, and patch
// for the percent sign.
// Returns nothing.
//
void STlib_initPercent(st_percent_t *p, int x, int y, const patch_t **pl, short *num, boolean *on, const patch_t *percent)
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}

/*
 * STlib_updatePercent()
 *
 * Draws a number/percent conditionally based on the widget's enable
 *
 * Passed a precent widget, the output color range, and a refresh flag
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps - const for pointer to the colour translation table
 */

void STlib_updatePercent(st_percent_t *per, int cm, int refresh)
{
    STlib_updateNum(&per->n, cm, refresh);
#if SCREENWIDTH != 240
    V_DrawPatchNoScale(per->n.x, per->n.y, per->p);// - Percentage is in the GBA Doom II Hud graphic ~Kippykip
#endif
}

//
// STlib_initMultIcon()
//
// Initialize a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys.
//
// Passed a st_multicon_t widget, the position, the graphic patches, a pointer
// to the numbers representing what to display, and pointer to the enable flag
// Returns nothing.
//
void STlib_initMultIcon(st_multicon_t *i, int x, int y, const patch_t **il, short *inum, boolean *on)
{
    i->x = x;
    i->y = y;
    i->oldinum = -1;
    i->inum = inum;
    i->on = on;
    i->p = il;
}

//
// STlib_updateMultIcon()
//
// Draw a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys. Displays each when the control
// numbers change or refresh is true
//
// Passed a st_multicon_t widget, and a refresh flag
// Returns nothing.
//
void STlib_updateMultIcon(st_multicon_t *mi, boolean refresh)
{
    if (!mi->p)
        return;

    if (*mi->inum != -1)  // killough 2/16/98: redraw only if != -1
    {
#ifdef CONFIG_DOOM_NO_COMPACT_PTR
        uint32_t temp_patch_addr;
        user_flash_read_game_resource(&temp_patch_addr, sizeof(temp_patch_addr), (uint32_t)&mi->p[*mi->inum]);
        // debugi("%s multiicon patch at %08x\r\n", __func__, temp_patch_addr);
        V_DrawPatchNoScale(mi->x, mi->y, (const patch_t*)temp_patch_addr);
#else
        // debugi("%s multiicon patch at %08x\r\n", __func__, (uint32_t)mi->p[*mi->inum]);
        V_DrawPatchNoScale(mi->x, mi->y, mi->p[*mi->inum]);
#endif

    }

    mi->oldinum = *mi->inum;

}

//
// STlib_initBinIcon()
//
// Initialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Passed a st_binicon_t widget, the position, the digit patches, a pointer
// to the flags representing what is displayed, and pointer to the enable flag
// Returns nothing.
//
void STlib_initBinIcon(st_binicon_t *b, int x, int y, const patch_t *i, boolean *val, boolean *on)
{
    b->x = x;
    b->y = y;
    b->oldval = 0;
    b->val = val;
    b->on = on;
    b->p = i;
}

//
// STlib_updateBinIcon()
//
// DInitialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Draw a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons that are present or not. Displays each
// when the control flag changes or refresh is true
//
// Passed a st_binicon_t widget, and a refresh flag
// Returns nothing.
//
void STlib_updateBinIcon(st_binicon_t *bi, boolean refresh)
{
    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        if (*bi->val)
            V_DrawPatch(bi->x, bi->y, ST_FG, bi->p);

        bi->oldval = *bi->val;
    }
}

void ST_refreshBackground(void)
{
    if (_g->st_statusbaron)
    {

#if SCREENWIDTH == 320
        V_DrawPatchNoScale(0, SCREENHEIGHT - ST_HEIGHT, (const patch_t *)gfx_stbar);
#else
        const unsigned int st_offset = ((SCREENHEIGHT - ST_SCALED_HEIGHT) * SCREENWIDTH_PHYSICAL);
        uint32_t *d = (uint32_t*) (((uint8_t*) _g->screens[0].data) + st_offset);
        uint32_t *s = (uint32_t*) gfx_stbar;
        for (int i = 0; i < gfx_stbar_len / sizeof(uint32_t); i++)
        {

            *d++ = *s++;
        }
#endif
    }
}
