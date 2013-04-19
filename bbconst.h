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

#ifndef BBCONST_H
#define BBCONST_H

#define SAMPLES_VERSION       "1.2.0"

#define SHIPS_SUBDIR          "bots"
#define STAGES_SUBDIR         "stages"
#define CACHE_SUBDIR          "cache"
#define TMP_SUBDIR            ".tmp"
#define ZIP_EXTENSION         ".tar.gz"
#define LUA_EXTENSION         ".lua"
#define STAGE_METAFILE        "stage.filename"
#define SHIP_METAFILE         "bot.filename"
#define MAX_FILENAME_LENGTH   1024

#define DEFAULT_STAGE_WIDTH   800
#define DEFAULT_STAGE_HEIGHT  600
#define SHIP_RADIUS           8
#define SHIP_SIZE             SHIP_RADIUS * 2
#define LASER_SPEED           25
#define LASER_HEAT            5
#define LASER_DAMAGE          4
#define TORPEDO_SPEED         12
#define TORPEDO_HEAT          100
#define TORPEDO_BLAST_RADIUS  100
#define TORPEDO_BLAST_FORCE   30
#define TORPEDO_BLAST_DAMAGE  30
#define DEFAULT_ENERGY        100
#define WALL_BOUNCE           0.5
#define MAX_THRUSTER_FORCE    1.0
#define COLLISION_FRAME       4
#define MAX_WALLS             1024
#define MAX_STARTS            1024
#define MAX_ZONES             1024
#define MAX_LASERS            8192
#define MAX_TORPEDOS          2048
#define MAX_STAGE_SHIPS       1024
#define MAX_STAGE_TEXTS       4096
#define MAX_USER_RECTANGLES   4096
#define MAX_USER_LINES        4096
#define MAX_USER_CIRCLES      4096
#define MAX_USER_TEXTS        4096
#define MAX_NAME_LENGTH       128
#define CPU_TIME_TICKS        1000
#define MAX_SCORE_STATS       1000

#if defined(_WIN32)
#define BB_DIRSEP      "\\"
#define BB_DIRSEP_CHR  '\\'
#else
#define BB_DIRSEP      "/"
#define BB_DIRSEP_CHR  '/'
#endif

#endif
