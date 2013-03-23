/*
  Copyright (C) 2012-2013 - Voidious

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BBGFX_H
#define BBGFX_H

#include "VG/vgu.h"
#include "bbconst.h"
#include "bbutil.h"
#include "stage.h"
#include "gfxeventhandler.h"

#define DRAW_SHIP_RADIUS          (SHIP_RADIUS - 1)
#define SHIP_OUTLINE_THICKNESS    1.7

#define SHIP_DOT_POSITION        SHIP_RADIUS * .39
#define SHIP_DOT_RADIUS          SHIP_RADIUS * .2
#define SHIP_DOT_SIZE            SHIP_DOT_RADIUS * 2
#define SHIP_DOT_FRAMES          240
#define LASER_THICKNESS          2
#define THRUSTER_THICKNESS       6
#define THRUSTER_LENGTH          12
#define ENERGY_THICKNESS         2
#define ENERGY_LENGTH            (SHIP_SIZE * 2)
#define THRUSTER_ZERO            (SHIP_RADIUS + 1.0) / (SHIP_RADIUS + THRUSTER_LENGTH)
#define TORPEDO_RADIUS           4
#define TORPEDO_SIZE             TORPEDO_RADIUS * 2
#define SHIP_DEATH_FRAMES        16
#define SHIP_DEATH_FRAME_LENGTH  2
#define SHIP_DEATH_TIME          (SHIP_DEATH_FRAMES * SHIP_DEATH_FRAME_LENGTH)
#define LASER_SPARK_LENGTH       8
#define LASER_SPARK_THICKNESS    1.5
#define LASER_SPARK_FRAMES       10
#define LASER_SPARK_TIME         LASER_SPARK_FRAMES
#define TORPEDO_BLAST_FRAMES     16
#define TORPEDO_BLAST_TIME       TORPEDO_BLAST_FRAMES

#define MIN_STAGE_TEXT_FONT_SIZE  8
#define MAX_STAGE_TEXT_FONT_SIZE  192

extern void initVgGfx(int screenWidth, int screenHeight, Stage *stage,
    Ship **ships, int numShips);
VGPaint createPaint(float *color);
extern void destroyVgGfx();
extern void drawGame(int screenWidth, int screenHeight, Stage *stage,
    Ship **ship, int numShips, int time, GfxEventHandler *gfxHandler);

#endif
