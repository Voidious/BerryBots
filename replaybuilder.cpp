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
  shipPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipAddData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipRemoveData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipTickData_ = new ReplayData(MAX_SHIP_TICK_CHUNKS);
  laserData_ = new ReplayData(MAX_LASER_CHUNKS);
  laserSparkData_ = new ReplayData(MAX_LASER_CHUNKS);
  torpedoData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoDebrisData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipDestroyData_ = new ReplayData(MAX_MISC_CHUNKS);
  textData_ = new ReplayData(MAX_MISC_CHUNKS);
}

ReplayBuilder::~ReplayBuilder() {
  delete shipsAlive_;
  delete shipPropertiesData_;
  delete shipAddData_;
  delete shipRemoveData_;
  delete shipTickData_;
  delete laserData_;
  delete laserSparkData_;
  delete torpedoData_;
  delete torpedoDebrisData_;
  delete shipDestroyData_;
  delete textData_;
}

// Ship properties format:  (variable)
// ship R | G | B | laser R | G | B | thruster R | G | B | nameLength | <name>
void ReplayBuilder::saveShipProperties(Ship *ship) {
  ShipProperties *properties = ship->properties;
  shipPropertiesData_->saveInt(properties->shipR);
  shipPropertiesData_->saveInt(properties->shipG);
  shipPropertiesData_->saveInt(properties->shipB);
  shipPropertiesData_->saveInt(properties->laserR);
  shipPropertiesData_->saveInt(properties->laserG);
  shipPropertiesData_->saveInt(properties->laserB);
  shipPropertiesData_->saveInt(properties->thrusterR);
  shipPropertiesData_->saveInt(properties->thrusterG);
  shipPropertiesData_->saveInt(properties->thrusterB);
  const char *name = ship->properties->name;
  int nameLength = (int) strlen(name);
  shipPropertiesData_->saveInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    shipPropertiesData_->saveInt((int) name[x]);
  }
}

// Ship add format:  (2)
// ship index | time
void ReplayBuilder::saveShipAdd(int shipIndex, int time) {
  shipAddData_->saveInt(shipIndex);
  shipAddData_->saveInt(time);
}

// Ship remove format:  (2)
// ship index | time
void ReplayBuilder::saveShipRemove(int shipIndex, int time) {
  shipRemoveData_->saveInt(shipIndex);
  shipRemoveData_->saveInt(time);
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
    shipTickData_->saveInt(round(ship->x * 10));
    shipTickData_->saveInt(round(ship->y * 10));
    shipTickData_->saveInt(
        round(normalAbsoluteAngle(ship->thrusterAngle) * 100));
    shipTickData_->saveInt(round(limit(0, ship->thrusterForce, 1) * 100));
    shipTickData_->saveInt(round(std::max(0.0, ship->energy) * 10));
  }
}

// Laser format:  (4)
// ship index | fire time | heading * 100 | duration
void ReplayBuilder::saveLaser(Laser *laser, int duration) {
  laserData_->saveInt(laser->shipIndex);
  laserData_->saveInt(laser->fireTime);
  laserData_->saveInt(round(laser->heading * 100));
  laserData_->saveInt(duration);
}

// Laser spark format:  (6)
// ship index | time | x * 10 | y * 10 | speed * 100 | heading * 100
void ReplayBuilder::saveLaserSpark(Laser *laser, double x, double y,
                                   double speed, double heading) {
  laserSparkData_->saveInt(laser->shipIndex);
  laserSparkData_->saveInt(laser->fireTime);
  laserSparkData_->saveInt(round(x * 10));
  laserSparkData_->saveInt(round(y * 10));
  laserSparkData_->saveInt(round(speed * 100));
  laserSparkData_->saveInt(round(heading * 100));
}

// Torpedo format:  (6)
// ship index | fire time | heading * 100 | duration | blast x * 10 | y * 10
void ReplayBuilder::saveTorpedo(Torpedo *torpedo, int fireTime, int duration,
                                double x, double y) {
  torpedoData_->saveInt(torpedo->shipIndex);
  torpedoData_->saveInt(fireTime);
  torpedoData_->saveInt(round(torpedo->heading * 100));
  torpedoData_->saveInt(duration);
  torpedoData_->saveInt(round(x * 10));
  torpedoData_->saveInt(round(y * 10));
}

// Torpedo debris format:  (7)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
void ReplayBuilder::saveTorpedoDebris(Ship *ship, int time, double dx,
                                      double dy, int parts) {
  torpedoDebrisData_->saveInt(ship->index);
  torpedoDebrisData_->saveInt(time);
  torpedoDebrisData_->saveInt(round(ship->x * 10));
  torpedoDebrisData_->saveInt(round(ship->y * 10));
  torpedoDebrisData_->saveInt(round(dx * 100));
  torpedoDebrisData_->saveInt(round(dy * 100));
  torpedoDebrisData_->saveInt(parts);
}

// Ship destroy format:  (4)
// ship index | time | x * 10 | y * 10
void ReplayBuilder::saveShipDestroy(Ship *ship, int time) {
  shipDestroyData_->saveInt(ship->index);
  shipDestroyData_->saveInt(time);
  shipDestroyData_->saveInt(round(ship->x * 10));
  shipDestroyData_->saveInt(round(ship->y * 10));
}

// Text format:  (variable)
// time | textLength | text | x * 10 | y * 10 | size | text R | G | B | A | duration
void ReplayBuilder::saveText(int time, const char *text, double x, double y,
                             int size, RgbaColor textColor, int duration) {
  textData_->saveInt(time);
  int textLength = (int) strlen(text);
  textData_->saveInt(textLength);
  for (int x = 0; x < textLength; x++) {
    textData_->saveInt((int) text[x]);
  }
  textData_->saveInt(round(x * 10));
  textData_->saveInt(round(y * 10));
  textData_->saveInt(size);
  textData_->saveInt(textColor.r);
  textData_->saveInt(textColor.g);
  textData_->saveInt(textColor.b);
  textData_->saveInt(textColor.a);
  textData_->saveInt(duration);
}

int ReplayBuilder::round(double f) {
  return floor(f + .5);
}

ReplayData::ReplayData(int maxChunks) {
  maxChunks_ = maxChunks;
  chunks_[0] = new ReplayChunk;
  numChunks_ = 1;
}

ReplayData::~ReplayData() {
  for (int x = 0; x < numChunks_; x++) {
    delete chunks_[x];
  }
}

void ReplayData::saveInt(int x) {
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
