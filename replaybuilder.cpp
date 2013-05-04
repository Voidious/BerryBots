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
  stagePropertiesData_ = new ReplayData(1);
  wallsData_ = new ReplayData(MAX_MISC_CHUNKS);
  zonesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipAddData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipRemoveData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipTickData_ = new ReplayData(MAX_SHIP_TICK_CHUNKS);
  laserStartData_ = new ReplayData(MAX_LASER_CHUNKS);
  laserEndData_ = new ReplayData(MAX_LASER_CHUNKS);
  laserSparkData_ = new ReplayData(MAX_LASER_CHUNKS);
  torpedoStartData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoEndData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoDebrisData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipDestroyData_ = new ReplayData(MAX_MISC_CHUNKS);
  textData_ = new ReplayData(MAX_MISC_CHUNKS);
}

ReplayBuilder::~ReplayBuilder() {
  delete shipsAlive_;
  delete stagePropertiesData_;
  delete wallsData_;
  delete zonesData_;
  delete shipPropertiesData_;
  delete shipAddData_;
  delete shipRemoveData_;
  delete shipTickData_;
  delete laserStartData_;
  delete laserEndData_;
  delete laserSparkData_;
  delete torpedoStartData_;
  delete torpedoEndData_;
  delete torpedoDebrisData_;
  delete shipDestroyData_;
  delete textData_;
}

// Stage size format:  (2)
// width | height
void ReplayBuilder::saveStageSize(int width, int height) {
  stagePropertiesData_->saveInt(width);
  stagePropertiesData_->saveInt(height);
}

// Wall format:  (4)
// left | bottom | width | height
void ReplayBuilder::saveWall(int left, int bottom, int width, int height) {
  wallsData_->saveInt(left);
  wallsData_->saveInt(bottom);
  wallsData_->saveInt(width);
  wallsData_->saveInt(height);
}

// Zone format:  (4)
// left | bottom | width | height
void ReplayBuilder::saveZone(int left, int bottom, int width, int height) {
  zonesData_->saveInt(left);
  zonesData_->saveInt(bottom);
  zonesData_->saveInt(width);
  zonesData_->saveInt(height);
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

// Laser start format:  (6)
// laser ID | ship index | fire time | x * 10 | y * 10 | heading * 100
void ReplayBuilder::saveLaserStart(Laser *laser) {
  laserStartData_->saveInt(laser->id);
  laserStartData_->saveInt(laser->shipIndex);
  laserStartData_->saveInt(laser->fireTime);
  laserStartData_->saveInt(round(laser->srcX * 10));
  laserStartData_->saveInt(round(laser->srcY * 10));
  laserStartData_->saveInt(round(laser->heading * 100));
}

// Laser end format:  (2)
// laser ID | end time
void ReplayBuilder::saveLaserEnd(Laser *laser, int time) {
  laserEndData_->saveInt(laser->id);
  laserEndData_->saveInt(time);
}

// Laser spark format:  (6)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100
void ReplayBuilder::saveLaserSpark(Laser *laser, int time, double x, double y,
                                   double dx, double dy) {
  laserSparkData_->saveInt(laser->shipIndex);
  laserSparkData_->saveInt(time);
  laserSparkData_->saveInt(round(x * 10));
  laserSparkData_->saveInt(round(y * 10));
  laserSparkData_->saveInt(round(dx * 100));
  laserSparkData_->saveInt(round(dy * 100));
}

// Torpedo start format:  (6)
// torpedo ID | ship index | fire time | x * 10 | y * 10 | heading * 100 
void ReplayBuilder::saveTorpedoStart(Torpedo *torpedo) {
  torpedoStartData_->saveInt(torpedo->id);
  torpedoStartData_->saveInt(torpedo->shipIndex);
  torpedoStartData_->saveInt(torpedo->fireTime);
  torpedoStartData_->saveInt(round(torpedo->srcX * 10));
  torpedoStartData_->saveInt(round(torpedo->srcY * 10));
  torpedoStartData_->saveInt(round(torpedo->heading * 100));
}

// Torpedo end format:  (4)
// torpedo ID | end time | x * 10 | y * 10
void ReplayBuilder::saveTorpedoEnd(Torpedo *torpedo, int time) {
  torpedoEndData_->saveInt(torpedo->id);
  torpedoEndData_->saveInt(time);
  torpedoEndData_->saveInt(round(torpedo->x * 10));
  torpedoEndData_->saveInt(round(torpedo->y * 10));
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
  chunks_ = new ReplayChunk*[maxChunks];
  maxChunks_ = maxChunks;
  chunks_[0] = new ReplayChunk;
  chunks_[0]->size = 0;
  numChunks_ = 1;
}

ReplayData::~ReplayData() {
  for (int x = 0; x < numChunks_; x++) {
    delete chunks_[x];
  }
  delete chunks_;
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

ReplayEventHandler::ReplayEventHandler(ReplayBuilder *replayBuilder) {
  replayBuilder_ = replayBuilder;
}

void ReplayEventHandler::handleShipFiredLaser(Ship *firingShip, Laser *laser) {
  replayBuilder_->saveLaserStart(laser);
}

void ReplayEventHandler::handleLaserDestroyed(Laser *laser, int time) {
  replayBuilder_->saveLaserEnd(laser, time);
}

void ReplayEventHandler::handleShipFiredTorpedo(Ship *firingShip,
                                                Torpedo *torpedo) {
  replayBuilder_->saveTorpedoStart(torpedo);
}

void ReplayEventHandler::handleTorpedoExploded(Torpedo *torpedo, int time) {
  replayBuilder_->saveTorpedoEnd(torpedo, time);
}

void ReplayEventHandler::handleShipDestroyed(
    Ship *destroyedShip, int time, Ship **destroyerShips, int numDestroyers) {
  replayBuilder_->saveShipDestroy(destroyedShip, time);
}

void ReplayEventHandler::handleLaserHitShip(Ship *srcShip, Ship *targetShip,
    Laser *laser, double dx, double dy, int time) {
  replayBuilder_->saveLaserSpark(
      laser, time, targetShip->x, targetShip->y, dx, dy);
}

void ReplayEventHandler::handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
    double dx, double dy, double hitAngle, double hitForce, double hitDamage,
    int time) {
  int parts = ceil((hitDamage / TORPEDO_BLAST_DAMAGE) * MAX_TORPEDO_SPARKS);
  replayBuilder_->saveTorpedoDebris(targetShip, time, dx, dy, parts);
}

void ReplayEventHandler::handleStageText(StageText *stageText) {
  RgbaColor textColor {stageText->textR, stageText->textG, stageText->textB,
                       stageText->textA};
  replayBuilder_->saveText(stageText->startTime, stageText->text, stageText->x,
      stageText->y, stageText->fontSize, textColor, stageText->drawTicks);
}
