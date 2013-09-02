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
#include <stdio.h>
#include <string.h>
#include "filemanager.h"
#include "bbutil.h"
#include "basedir.h"
#include "replaybuilder.h"

ReplayBuilder::ReplayBuilder(int numShips, const char *templateDir) {
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
  torpedoBlastData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoDebrisData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipDestroyData_ = new ReplayData(MAX_MISC_CHUNKS);
  textData_ = new ReplayData(MAX_TEXT_CHUNKS);
  numTexts_ = 0;

  if (templateDir == 0) {
    templatePath_ = 0;
  } else {
    FileManager fileManager;
    templatePath_ = fileManager.getFilePath(templateDir, REPLAY_TEMPLATE);
  }
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
  delete torpedoBlastData_;
  delete torpedoDebrisData_;
  delete shipDestroyData_;
  delete textData_;
  if (templatePath_ != 0) {
    delete templatePath_;
  }
}

// Stage size format:  (2)
// width | height
void ReplayBuilder::addStageSize(int width, int height) {
  stagePropertiesData_->addInt(width);
  stagePropertiesData_->addInt(height);
}

// Wall format:  (4)
// left | bottom | width | height
void ReplayBuilder::addWall(int left, int bottom, int width, int height) {
  wallsData_->addInt(left);
  wallsData_->addInt(bottom);
  wallsData_->addInt(width);
  wallsData_->addInt(height);
}

// Zone format:  (4)
// left | bottom | width | height
void ReplayBuilder::addZone(int left, int bottom, int width, int height) {
  zonesData_->addInt(left);
  zonesData_->addInt(bottom);
  zonesData_->addInt(width);
  zonesData_->addInt(height);
}

// Ship properties format:  (variable)
// ship R | G | B | laser R | G | B | thruster R | G | B | nameLength | <name>
void ReplayBuilder::addShipProperties(Ship *ship) {
  ShipProperties *properties = ship->properties;
  shipPropertiesData_->addInt(properties->shipR);
  shipPropertiesData_->addInt(properties->shipG);
  shipPropertiesData_->addInt(properties->shipB);
  shipPropertiesData_->addInt(properties->laserR);
  shipPropertiesData_->addInt(properties->laserG);
  shipPropertiesData_->addInt(properties->laserB);
  shipPropertiesData_->addInt(properties->thrusterR);
  shipPropertiesData_->addInt(properties->thrusterG);
  shipPropertiesData_->addInt(properties->thrusterB);
  const char *name = ship->properties->name;
  int nameLength = (int) strlen(name);
  shipPropertiesData_->addInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    shipPropertiesData_->addInt((int) name[x]);
  }
}

// Ship add format:  (2)
// ship index | time
void ReplayBuilder::addShip(int shipIndex, int time) {
  shipAddData_->addInt(shipIndex);
  shipAddData_->addInt(time);
}

// Ship remove format:  (2)
// ship index | time
void ReplayBuilder::removeShip(int shipIndex, int time) {
  shipRemoveData_->addInt(shipIndex);
  shipRemoveData_->addInt(time);
}

// Ship tick format:  (5)
// x * 10 | y * 10 | thruster angle * 100 | force * 100 | energy * 10
void ReplayBuilder::addShipStates(Ship **ships, int time) {
  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    if (shipsAlive_[x] != ship->alive) {
      if (ship->alive) {
        addShip(ship->index, time);
      } else {
        removeShip(ship->index, time);
      }
      shipsAlive_[x] = ship->alive;
    }
  }

  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      shipTickData_->addInt(round(ship->x * 10));
      shipTickData_->addInt(round(ship->y * 10));
      shipTickData_->addInt(
          round(normalAbsoluteAngle(ship->thrusterAngle) * 100));
      shipTickData_->addInt(round(limit(0, ship->thrusterForce, 1) * 100));
      shipTickData_->addInt(round(std::max(0.0, ship->energy) * 10));
    }
  }
}

// Laser start format:  (6)
// laser ID | ship index | fire time | x * 10 | y * 10 | heading * 100
void ReplayBuilder::addLaserStart(Laser *laser) {
  laserStartData_->addInt(laser->id);
  laserStartData_->addInt(laser->shipIndex);
  laserStartData_->addInt(laser->fireTime);
  laserStartData_->addInt(round(laser->srcX * 10));
  laserStartData_->addInt(round(laser->srcY * 10));
  laserStartData_->addInt(round(laser->heading * 100));
}

// Laser end format:  (2)
// laser ID | end time
void ReplayBuilder::addLaserEnd(Laser *laser, int time) {
  laserEndData_->addInt(laser->id);
  laserEndData_->addInt(time);
}

// Laser spark format:  (6)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100
void ReplayBuilder::addLaserSpark(Laser *laser, int time, double x, double y,
                                   double dx, double dy) {
  laserSparkData_->addInt(laser->shipIndex);
  laserSparkData_->addInt(time);
  laserSparkData_->addInt(round(x * 10));
  laserSparkData_->addInt(round(y * 10));
  laserSparkData_->addInt(round(dx * 100));
  laserSparkData_->addInt(round(dy * 100));
}

// Torpedo start format:  (6)
// torpedo ID | ship index | fire time | x * 10 | y * 10 | heading * 100 
void ReplayBuilder::addTorpedoStart(Torpedo *torpedo) {
  torpedoStartData_->addInt(torpedo->id);
  torpedoStartData_->addInt(torpedo->shipIndex);
  torpedoStartData_->addInt(torpedo->fireTime);
  torpedoStartData_->addInt(round(torpedo->srcX * 10));
  torpedoStartData_->addInt(round(torpedo->srcY * 10));
  torpedoStartData_->addInt(round(torpedo->heading * 100));
}

// Torpedo end format:  (2)
// torpedo ID | end time
void ReplayBuilder::addTorpedoEnd(Torpedo *torpedo, int time) {
  torpedoEndData_->addInt(torpedo->id);
  torpedoEndData_->addInt(time);
}

// Torpedo blast format:  (3)
// time | x * 10 | y * 10
void ReplayBuilder::addTorpedoBlast(Torpedo *torpedo, int time) {
  torpedoBlastData_->addInt(time);
  torpedoBlastData_->addInt(round(torpedo->x * 10));
  torpedoBlastData_->addInt(round(torpedo->y * 10));
}

// Torpedo debris format:  (7)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
void ReplayBuilder::addTorpedoDebris(Ship *ship, int time, double dx,
                                      double dy, int parts) {
  torpedoDebrisData_->addInt(ship->index);
  torpedoDebrisData_->addInt(time);
  torpedoDebrisData_->addInt(round(ship->x * 10));
  torpedoDebrisData_->addInt(round(ship->y * 10));
  torpedoDebrisData_->addInt(round(dx * 100));
  torpedoDebrisData_->addInt(round(dy * 100));
  torpedoDebrisData_->addInt(parts);
}

// Ship destroy format:  (4)
// ship index | time | x * 10 | y * 10
void ReplayBuilder::addShipDestroy(Ship *ship, int time) {
  shipDestroyData_->addInt(ship->index);
  shipDestroyData_->addInt(time);
  shipDestroyData_->addInt(round(ship->x * 10));
  shipDestroyData_->addInt(round(ship->y * 10));
}

// Text format:  (variable)
// time | textLength | text | x * 10 | y * 10 | size | text R | G | B | A | duration
void ReplayBuilder::addText(int time, const char *text, double x, double y,
                            int size, RgbaColor textColor, int duration) {
  textData_->addInt(time);
  int textLength = (int) strlen(text);
  textData_->addInt(textLength);
  for (int z = 0; z < textLength; z++) {
    textData_->addInt((int) text[z]);
  }
  textData_->addInt(round(x * 10));
  textData_->addInt(round(y * 10));
  textData_->addInt(size);
  textData_->addInt(textColor.r);
  textData_->addInt(textColor.g);
  textData_->addInt(textColor.b);
  textData_->addInt(textColor.a);
  textData_->addInt(duration);
  numTexts_++;
}

int ReplayBuilder::round(double f) {
  return floor(f + .5);
}

// Format of saved replay file:
// | replay version
// | stage width | stage height | num walls | <walls> | num zones | <zones>
// | num ships | <ship properties> | num ship adds | <ship adds>
// | num ship removes | <ship removes> | num ship ticks | <ship ticks>
// | num laser starts | <laser starts> | num laser ends | <laser ends>
// | num laser sparks | <laser sparks>
// | num torpedo starts | <torpedo starts> | num torpedo ends | <torpedo ends>
// | num torpedo blasts | <torpedo blasts> | num torpedo debris | <torpedo debris>
// | num ship destroys | <ship destroys> | num texts | <texts>
void ReplayBuilder::saveReplay(const char *filename) {
  // TODO: throw exceptions for failing to save replay, don't silently fail

  FileManager fileManager;
  char *replayTemplate;
  try {
    replayTemplate = fileManager.readFile(templatePath_);
  } catch (FileNotFoundException *e) {
    delete e;
    replayTemplate = 0;
  }

  if (replayTemplate != 0) {
    std::string replayHtml;
    const char *phStart = strstr(replayTemplate, REPLAY_DATA_PLACEHOLDER);
    if (phStart == NULL) {
      return;
    }
    replayHtml.append(replayTemplate, (phStart - replayTemplate));

    char *replayData = buildReplayDataString();
    replayHtml.append(replayData);
    delete replayData;

    const char *phEnd = &(phStart[strlen(REPLAY_DATA_PLACEHOLDER)]);
    replayHtml.append(phEnd);

    char *filePath = fileManager.getFilePath(getReplaysDir().c_str(), filename);
    char *absFilename = fileManager.getAbsFilePath(filePath);
    delete filePath;
    fileManager.writeFile(absFilename, replayHtml.c_str());
    delete absFilename;
  }
}

char* ReplayBuilder::buildReplayDataString() {
/*
  FILE *f = fopen(absFilename, "wb");

  int v = REPLAY_VERSION;
  fwrite(&v, sizeof(int), 1, f);

  stagePropertiesData_->writeChunks(f);

  int numWalls = wallsData_->getSize() / 4;
  fwrite(&numWalls, sizeof(int), 1, f);
  wallsData_->writeChunks(f);

  int numZones = zonesData_->getSize() / 4;
  fwrite(&numZones, sizeof(int), 1, f);
  zonesData_->writeChunks(f);

  fwrite(&numShips_, sizeof(int), 1, f);
  shipPropertiesData_->writeChunks(f);

  int numAddShips = shipAddData_->getSize() / 2;
  fwrite(&numAddShips, sizeof(int), 1, f);
  shipAddData_->writeChunks(f);

  int numRemoveShips = shipRemoveData_->getSize() / 2;
  fwrite(&numRemoveShips, sizeof(int), 1, f);
  shipRemoveData_->writeChunks(f);

  int numShipTicks = shipTickData_->getSize() / 5;
  fwrite(&numShipTicks, sizeof(int), 1, f);
  shipTickData_->writeChunks(f);

  int numLaserStarts = laserStartData_->getSize() / 6;
  fwrite(&numLaserStarts, sizeof(int), 1, f);
  laserStartData_->writeChunks(f);

  int numLaserEnds = laserEndData_->getSize() / 2;
  fwrite(&numLaserEnds, sizeof(int), 1, f);
  laserEndData_->writeChunks(f);

  int numLaserSparks = laserSparkData_->getSize() / 6;
  fwrite(&numLaserSparks, sizeof(int), 1, f);
  laserSparkData_->writeChunks(f);

  int numTorpedoStarts = torpedoStartData_->getSize() / 6;
  fwrite(&numTorpedoStarts, sizeof(int), 1, f);
  torpedoStartData_->writeChunks(f);

  int numTorpedoEnds = torpedoEndData_->getSize() / 2;
  fwrite(&numTorpedoEnds, sizeof(int), 1, f);
  torpedoEndData_->writeChunks(f);

  int numTorpedoBlasts = torpedoBlastData_->getSize() / 3;
  fwrite(&numTorpedoBlasts, sizeof(int), 1, f);
  torpedoBlastData_->writeChunks(f);

  int numTorpedoDebris = torpedoDebrisData_->getSize() / 7;
  fwrite(&numTorpedoDebris, sizeof(int), 1, f);
  torpedoDebrisData_->writeChunks(f);

  int numShipDestroys = shipDestroyData_->getSize() / 4;
  fwrite(&numShipDestroys, sizeof(int), 1, f);
  shipDestroyData_->writeChunks(f);
  
  fwrite(&numTexts_, sizeof(int), 1, f);
  textData_->writeChunks(f);

  fclose(f);
*/
  char *replayDataString = new char[1];
  replayDataString[0] = '\0';
  return replayDataString;
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

void ReplayData::addInt(int x) {
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

int ReplayData::getSize() {
  return ((numChunks_ - 1) * CHUNK_SIZE) + chunks_[numChunks_ - 1]->size;
}

void ReplayData::writeChunks(FILE *f) {
  for (int x = 0; x < numChunks_; x++) {
    fwrite(chunks_[x], sizeof(int), chunks_[x]->size, f);
  }
}

ReplayEventHandler::ReplayEventHandler(ReplayBuilder *replayBuilder) {
  replayBuilder_ = replayBuilder;
}

void ReplayEventHandler::handleShipFiredLaser(Ship *firingShip, Laser *laser) {
  replayBuilder_->addLaserStart(laser);
}

void ReplayEventHandler::handleLaserDestroyed(Laser *laser, int time) {
  replayBuilder_->addLaserEnd(laser, time);
}

void ReplayEventHandler::handleShipFiredTorpedo(Ship *firingShip,
                                                Torpedo *torpedo) {
  replayBuilder_->addTorpedoStart(torpedo);
}

void ReplayEventHandler::handleTorpedoDestroyed(Torpedo *torpedo, int time) {
  replayBuilder_->addTorpedoEnd(torpedo, time);
}

void ReplayEventHandler::handleTorpedoExploded(Torpedo *torpedo, int time) {
  replayBuilder_->addTorpedoEnd(torpedo, time);
  replayBuilder_->addTorpedoBlast(torpedo, time);
}

void ReplayEventHandler::handleShipDestroyed(
    Ship *destroyedShip, int time, Ship **destroyerShips, int numDestroyers) {
  replayBuilder_->addShipDestroy(destroyedShip, time);
}

void ReplayEventHandler::handleLaserHitShip(Ship *srcShip, Ship *targetShip,
    Laser *laser, double dx, double dy, int time) {
  replayBuilder_->addLaserSpark(
      laser, time, targetShip->x, targetShip->y, dx, dy);
}

void ReplayEventHandler::handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
    double dx, double dy, double hitAngle, double hitForce, double hitDamage,
    int time) {
  int parts = ceil((hitDamage / TORPEDO_BLAST_DAMAGE) * MAX_TORPEDO_SPARKS);
  replayBuilder_->addTorpedoDebris(targetShip, time, dx, dy, parts);
}

void ReplayEventHandler::handleStageText(StageText *stageText) {
  RgbaColor textColor;
  textColor.r = stageText->textR;
  textColor.g = stageText->textG;
  textColor.b = stageText->textB;
  textColor.a = stageText->textA;
  
  replayBuilder_->addText(stageText->startTime, stageText->text, stageText->x,
      stageText->y, stageText->fontSize, textColor, stageText->drawTicks);
}
