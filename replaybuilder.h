/*
  Copyright (C) 2013 - Voidious

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

#include "bbutil.h"

#define CHUNK_SIZE            (1024 * 1024 / 8)  // 500 kb of ints
#define MAX_SHIP_TICK_CHUNKS  40                 // 20 megs
#define MAX_LASER_CHUNKS      40                 // 20 megs lasers/sparks
#define MAX_MISC_CHUNKS       2                  // 1 megs each

typedef struct {
  int data[CHUNK_SIZE];
  int size;
} ReplayChunk;

class ChunkSet {
  ReplayChunk* chunks_[MAX_MISC_CHUNKS];
  int numChunks_;
  int maxChunks_;

  public:
    ChunkSet(int maxChunks);
    ~ChunkSet();
    void saveInt(int x);
};

class ReplayBuilder {
  bool *shipsAlive_;
  int numShips_;
  ChunkSet *shipPropertiesChunks_;
  ChunkSet *shipAddChunks_;
  ChunkSet *shipRemoveChunks_;
  ChunkSet *shipTickChunks_;
  ChunkSet *laserChunks_;
  ChunkSet *laserSparkChunks_;
  ChunkSet *torpedoChunks_;
  ChunkSet *torpedoDebrisChunks_;
  ChunkSet *shipDestroyChunks_;
  ChunkSet *textChunks_;
  
  public:
    ReplayBuilder(int numShips);
    ~ReplayBuilder();
    void saveShipProperties(Ship *ship);
    void saveShipStates(Ship **ships, int time);
    void saveLaser(Laser *laser, int duration);
    void saveLaserSpark(Laser *laser, double x, double y, double speed,
                        double heading);
    void saveTorpedo(Torpedo *torpedo, int fireTime, int duration, double x,
                     double y);
    void saveTorpedoDebris(Ship *ship, int time, double dx, double dy,
                           int parts);
    void saveShipDestroy(Ship *ship, int time);
    void saveText(int time, const char *text, double x, double y, int size,
                  RgbaColor textColor, int duration);

  private:
    void saveShipAdd(int shipIndex, int time);
    void saveShipRemove(int shipIndex, int time);
    int round(double f);
};
