/**
 *  Display utility functions for Doom Port to nRF52840 MCU.
 *  It also contains the interrupt-assisted DMA update of the display
 *  and double buffer display data structure.
 *
 *  Copyright (C) 2021 by Nicola Wrachien (next-hack in the comments)
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
 *  Interrupt assisted DMA-based display update engine, with double buffering
 *  256-color functions to print general purpose texts.
 *
 */
#ifndef SRC_GRAPHICS_H_
#define SRC_GRAPHICS_H_
#include <stdbool.h>
#include <stdint.h>

#include "user_display.h"

// #define PIXELS_PER_DMA_LINE 256
#define PIXELS_PER_DMA_LINE USER_SCREEN_WIDTH
#define NUMBER_OF_DMA_LINES (USER_SCREEN_HEIGHT * USER_SCREEN_WIDTH / PIXELS_PER_DMA_LINE)

typedef struct
{
    uint16_t * pPalette;
    uint16_t displayDmaLineBuffer[2][PIXELS_PER_DMA_LINE];
    uint8_t displayFrameBuffer[2][USER_SCREEN_HEIGHT * USER_SCREEN_WIDTH];
    uint8_t currentDisplayDmaLineBuffer;        // small buffer to speed up DMA send
    uint8_t * currentDmaFrameBuffer;            // points to data to be converted, using pPalette on the displayDmaBuffer
    uint8_t displayDmaLineBuffersSent; // how many lines we sent
    uint8_t workingBuffer;
    uint8_t displayMode;
    volatile uint8_t dmaBusy;
#if NEW_DISPLAY_UPDATE_WAY
    volatile uint8_t doNotDisableShorts;
    volatile uint8_t nextBufferIndex;
#endif
} displayData_t;
extern displayData_t displayData;
void displayPrintf(int x, int y, const char * format, ...);
void startDisplayRefresh(uint8_t bufferNumber);
void setDisplayPen(int color, int background);
void clearScreen4bpp();
void displayPrintln(bool update, const char * format, ...);
void initGraphics();
#endif /* SRC_GRAPHICS_H_ */