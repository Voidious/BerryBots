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

#include <math.h>
#include <algorithm>
#include "bbutil.h"
#include "replaybuilder.h"

ReplayBuilder::ReplayBuilder(int numShips) {
  shipsAlive_ = new bool[numShips];
  numShips_ = numShips;
  for (int x = 0; x < numShips; x++) {
    shipsAlive_[x] = false;
  }
  shipPropertiesChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  shipAddChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  shipRemoveChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  shipTickChunks_ = new ChunkSet(MAX_SHIP_TICK_CHUNKS);
  laserChunks_ = new ChunkSet(MAX_LASER_CHUNKS);
  laserSparkChunks_ = new ChunkSet(MAX_LASER_CHUNKS);
  torpedoChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  torpedoDebrisChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  shipDestroyChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
  textChunks_ = new ChunkSet(MAX_MISC_CHUNKS);
}

ReplayBuilder::~ReplayBuilder() {
  delete shipsAlive_;
  delete shipPropertiesChunks_;
  delete shipAddChunks_;
  delete shipRemoveChunks_;
  delete shipTickChunks_;
  delete laserChunks_;
  delete laserSparkChunks_;
  delete torpedoChunks_;
  delete torpedoDebrisChunks_;
  delete shipDestroyChunks_;
  delete textChunks_;
}

// Ship properties format:  (variable)
// ship R | G | B | laser R | G | B | thruster R | G | B | nameLength | <name>
void ReplayBuilder::saveShipProperties(Ship *ship) {
  ShipProperties *properties = ship->properties;
  shipPropertiesChunks_->saveInt(properties->shipR);
  shipPropertiesChunks_->saveInt(properties->shipG);
  shipPropertiesChunks_->saveInt(properties->shipB);
  shipPropertiesChunks_->saveInt(properties->laserR);
  shipPropertiesChunks_->saveInt(properties->laserG);
  shipPropertiesChunks_->saveInt(properties->laserB);
  shipPropertiesChunks_->saveInt(properties->thrusterR);
  shipPropertiesChunks_->saveInt(properties->thrusterG);
  shipPropertiesChunks_->saveInt(properties->thrusterB);
  const char *name = ship->properties->name;
  int nameLength = (int) strlen(name);
  shipPropertiesChunks_->saveInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    shipPropertiesChunks_->saveInt((int) name[x]);
  }
}

// Ship add format:  (2)
// ship index | time
void ReplayBuilder::saveShipAdd(int shipIndex, int time) {
  shipAddChunks_->saveInt(shipIndex);
  shipAddChunks_->saveInt(time);
}

// Ship remove format:  (2)
// ship index | time
void ReplayBuilder::saveShipRemove(int shipIndex, int time) {
  shipRemoveChunks_->saveInt(shipIndex);
  shipRemoveChunks_->saveInt(time);
}

// Ship tick format:  (5)
// x * 10 | y * 10 | thruster angle * 100 | force * 100 | energy * 10
void ReplayBuilder::saveShipStates(Ship **ships, int time) {
  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    if (shipsAlive_[x] != ship->alive) {
      if (ship->alive) {
        saveShipAdd(ship->index, time);
      } else {
        saveShipRemove(ship->index, time);
      }
      shipsAlive_[x] = ship->alive;
    }
  }

  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    shipTickChunks_->saveInt(round(ship->x * 10));
    shipTickChunks_->saveInt(round(ship->y * 10));
    shipTickChunks_->saveInt(
        round(normalAbsoluteAngle(ship->thrusterAngle) * 100));
    shipTickChunks_->saveInt(round(limit(0, ship->thrusterForce, 1) * 100));
    shipTickChunks_->saveInt(round(std::max(0.0, ship->energy) * 10));
  }
}

// Laser format:  (4)
// ship index | fire time | heading * 100 | duration
void ReplayBuilder::saveLaser(Laser *laser, int duration) {
  laserChunks_->saveInt(laser->shipIndex);
  laserChunks_->saveInt(laser->fireTime);
  laserChunks_->saveInt(round(laser->heading * 100));
  laserChunks_->saveInt(duration);
}

// Laser spark format:  (6)
// ship index | time | x * 10 | y * 10 | speed * 100 | heading * 100
void ReplayBuilder::saveLaserSpark(Laser *laser, double x, double y,
                                   double speed, double heading) {
  laserSparkChunks_->saveInt(laser->shipIndex);
  laserSparkChunks_->saveInt(laser->fireTime);
  laserSparkChunks_->saveInt(round(x * 10));
  laserSparkChunks_->saveInt(round(y * 10));
  laserSparkChunks_->saveInt(round(speed * 100));
  laserSparkChunks_->saveInt(round(heading * 100));
}

// Torpedo format:  (6)
// ship index | fire time | heading * 100 | duration | blast x * 10 | y * 10
void ReplayBuilder::saveTorpedo(Torpedo *torpedo, int fireTime, int duration,
                                double x, double y) {
  torpedoChunks_->saveInt(torpedo->shipIndex);
  torpedoChunks_->saveInt(fireTime);
  torpedoChunks_->saveInt(round(torpedo->heading * 100));
  torpedoChunks_->saveInt(duration);
  torpedoChunks_->saveInt(round(x * 10));
  torpedoChunks_->saveInt(round(y * 10));
}

// Torpedo debris format:  (7)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
void ReplayBuilder::saveTorpedoDebris(Ship *ship, int time, double dx,
                                      double dy, int parts) {
  torpedoDebrisChunks_->saveInt(ship->index);
  torpedoDebrisChunks_->saveInt(time);
  torpedoDebrisChunks_->saveInt(round(ship->x * 10));
  torpedoDebrisChunks_->saveInt(round(ship->y * 10));
  torpedoDebrisChunks_->saveInt(round(dx * 100));
  torpedoDebrisChunks_->saveInt(round(dy * 100));
  torpedoDebrisChunks_->saveInt(parts);
}

// Ship destroy format:  (4)
// ship index | time | x * 10 | y * 10
void ReplayBuilder::saveShipDestroy(Ship *ship, int time) {
  shipDestroyChunks_->saveInt(ship->index);
  shipDestroyChunks_->saveInt(time);
  shipDestroyChunks_->saveInt(round(ship->x * 10));
  shipDestroyChunks_->saveInt(round(ship->y * 10));
}

// Text format:  (variable)
// time | textLength | text | x * 10 | y * 10 | size | text R | G | B | A | duration
void ReplayBuilder::saveText(int time, const char *text, double x, double y,
                             int size, RgbaColor textColor, int duration) {
  textChunks_->saveInt(time);
  int textLength = (int) strlen(text);
  textChunks_->saveInt(textLength);
  for (int x = 0; x < textLength; x++) {
    textChunks_->saveInt((int) text[x]);
  }
  textChunks_->saveInt(round(x * 10));
  textChunks_->saveInt(round(y * 10));
  textChunks_->saveInt(size);
  textChunks_->saveInt(textColor.r);
  textChunks_->saveInt(textColor.g);
  textChunks_->saveInt(textColor.b);
  textChunks_->saveInt(textColor.a);
  textChunks_->saveInt(duration);
}

int ReplayBuilder::round(double f) {
  return floor(f + .5);
}

ChunkSet::ChunkSet(int maxChunks) {
  maxChunks_ = maxChunks;
  chunks_[0] = new ReplayChunk;
  numChunks_ = 1;
}

ChunkSet::~ChunkSet() {
  for (int x = 0; x < numChunks_; x++) {
    delete chunks_[x];
  }
}

void ChunkSet::saveInt(int x) {
  ReplayChunk *chunk = chunks_[numChunks_ - 1];
  if (chunk->size == CHUNK_SIZE) {
    if (numChunks_ == maxChunks_) {
      return;
    }

    chunk = chunks_[numChunks_++] = new ReplayChunk;
    chunk->size = 0;
  }
  chunk->data[chunk->size++] = x;
}
