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

#include "sensorhandler.h"

SensorHandler::SensorHandler(Team **teams, int numTeams, bool **teamVision) {
  teams_ = teams;
  numTeams_ = numTeams;
  teamVision_ = teamVision;
  hitByShips_ = new HitByShip**[numTeams_];
  numHitByShips_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    hitByShips_[x] = new HitByShip*[MAX_HIT_BY_SHIPS];
    numHitByShips_[x] = 0;
  }

  hitByLasers_ = new HitByLaser**[numTeams_];
  numHitByLasers_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    hitByLasers_[x] = new HitByLaser*[MAX_HIT_BY_LASERS];
    numHitByLasers_[x] = 0;
  }

  hitByTorpedos_ = new HitByTorpedo**[numTeams_];
  numHitByTorpedos_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    hitByTorpedos_[x] = new HitByTorpedo*[MAX_HIT_BY_TORPEDOS];
    numHitByTorpedos_[x] = 0;
  }

  hitWalls_ = new ShipHitWall**[numTeams_];
  numHitWalls_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    hitWalls_[x] = new ShipHitWall*[MAX_HIT_WALLS];
    numHitWalls_[x] = 0;
  }

  shipDestroyeds_ = new ShipDestroyed**[numTeams_];
  numShipDestroyeds_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    shipDestroyeds_[x] = new ShipDestroyed*[MAX_SHIP_DESTROYEDS];
    numShipDestroyeds_[x] = 0;
  }

  shipFiredLasers_ = new ShipFiredLaser**[numTeams_];
  numShipFiredLasers_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    shipFiredLasers_[x] = new ShipFiredLaser*[MAX_SHIP_FIRED_LASERS];
    numShipFiredLasers_[x] = 0;
  }

  shipFiredTorpedos_ = new ShipFiredTorpedo**[numTeams_];
  numShipFiredTorpedos_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    shipFiredTorpedos_[x] = new ShipFiredTorpedo*[MAX_SHIP_FIRED_TORPEDOS];
    numShipFiredTorpedos_[x] = 0;
  }

  laserHitShips_ = new LaserHitShip**[numTeams_];
  numLaserHitShips_ = new int[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    laserHitShips_[x] = new LaserHitShip*[MAX_LASER_HIT_SHIPS];
    numLaserHitShips_[x] = 0;
  }

  numStageShipHitShips_ = 0;
  numStageLaserHitShips_ = 0;
  numStageTorpedoHitShips_ = 0;
  numStageShipHitWalls_ = 0;
  numStageShipDestroyeds_ = 0;
  numStageShipFiredLasers_ = 0;
  numStageShipFiredTorpedos_ = 0;
}

void SensorHandler::handleLaserHitShip(Ship *srcShip, Ship *targetShip,
    double dx, double dy, double laserX, double laserY, double laserHeading,
    int time) {
  int targetShipIndex = targetShip->index;
  int teamIndex = targetShip->teamIndex;
  if (numHitByLasers_[teamIndex] < MAX_HIT_BY_LASERS) {
    HitByLaser *hitByLaser = new HitByLaser;
    hitByLaser->time = time;
    hitByLaser->targetShipIndex = targetShipIndex;
    hitByLaser->laserX = laserX;
    hitByLaser->laserY = laserY;
    hitByLaser->laserHeading = laserHeading;
    hitByLasers_[teamIndex][numHitByLasers_[teamIndex]++] = hitByLaser;
  }

  for (int x = 0; x < numTeams_; x++) {
    if (x != teamIndex && teamVision_[x][targetShipIndex]) {
      if (numLaserHitShips_[x] < MAX_LASER_HIT_SHIPS) {
        LaserHitShip *laserHitShip = new LaserHitShip;
        laserHitShip->time = time;
        laserHitShip->targetShipIndex = targetShipIndex;
        laserHitShip->shipX = targetShip->x;
        laserHitShip->shipY = targetShip->y;
        laserHitShips_[x][numLaserHitShips_[x]++] = laserHitShip;
      }
    }
  }

  if (numStageLaserHitShips_ < MAX_LASER_HIT_SHIPS) {
    StageLaserHitShip *laserHitShip = new StageLaserHitShip;
    laserHitShip->time = time;
    laserHitShip->srcShipIndex = srcShip->index;
    laserHitShip->targetShipIndex = targetShipIndex;
    laserHitShip->laserX = laserX;
    laserHitShip->laserY = laserY;
    laserHitShip->laserHeading = laserHeading;
    stageLaserHitShips_[numStageLaserHitShips_++] = laserHitShip;
  }
}

void SensorHandler::handleTorpedoExploded(double x, double y, int time) {
  // no stage event for this for now, just HitByTorpedo
}

void SensorHandler::handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
    double dx, double dy, double hitAngle, double hitForce, double hitDamage,
    int time) {
  int targetShipIndex = targetShip->index;
  int teamIndex = targetShip->teamIndex;
  if (numHitByTorpedos_[teamIndex] < MAX_HIT_BY_TORPEDOS) {
    HitByTorpedo *hitByTorpedo = new HitByTorpedo;
    hitByTorpedo->time = time;
    hitByTorpedo->targetShipIndex = targetShipIndex;
    hitByTorpedo->hitAngle = hitAngle;
    hitByTorpedo->hitForce = hitForce;
    hitByTorpedo->hitDamage = hitDamage;
    hitByTorpedos_[teamIndex][numHitByTorpedos_[teamIndex]++] = hitByTorpedo;
  }

  if (numStageTorpedoHitShips_ < MAX_HIT_BY_TORPEDOS) {
    StageTorpedoHitShip *torpedoHitShip = new StageTorpedoHitShip;
    torpedoHitShip->time = time;
    torpedoHitShip->srcShipIndex = srcShip->index;
    torpedoHitShip->targetShipIndex = targetShip->index;
    torpedoHitShip->hitAngle = hitAngle;
    torpedoHitShip->hitForce = hitForce;
    torpedoHitShip->hitDamage = hitDamage;
    stageTorpedoHitShips_[numStageTorpedoHitShips_++] = torpedoHitShip;
  }
}

void SensorHandler::handleShipHitShip(Ship *hittingShip, Ship *targetShip,
    double inAngle, double inForce, double outAngle, double outForce,
    int time) {
  int teamIndex = targetShip->teamIndex;
  if (numHitByShips_[teamIndex] < MAX_HIT_BY_SHIPS) {
    HitByShip *hitByShip = new HitByShip;
    hitByShip->time = time;
    hitByShip->targetShipIndex = targetShip->index;
    hitByShip->targetX = targetShip->x;
    hitByShip->targetY = targetShip->y;
    hitByShip->hittingShipIndex = hittingShip->index;
    hitByShip->hittingX = hittingShip->x;
    hitByShip->hittingY = hittingShip->y;
    hitByShip->inAngle = inAngle;
    hitByShip->inForce = inForce;
    hitByShip->outAngle = outAngle;
    hitByShip->outForce = outForce;
    hitByShips_[teamIndex][numHitByShips_[teamIndex]++] = hitByShip;
  }

  if (numStageShipHitShips_ < MAX_SHIP_HIT_SHIPS) {
    ShipHitShip *shipHitShip = new ShipHitShip;
    shipHitShip->time = time;
    shipHitShip->targetShipIndex = targetShip->index;
    shipHitShip->targetX = targetShip->x;
    shipHitShip->targetY = targetShip->y;
    shipHitShip->hittingShipIndex = hittingShip->index;
    shipHitShip->hittingX = hittingShip->x;
    shipHitShip->hittingY = hittingShip->y;
    shipHitShip->inAngle = inAngle;
    shipHitShip->inForce = inForce;
    shipHitShip->outAngle = outAngle;
    shipHitShip->outForce = outForce;
    stageShipHitShips_[numStageShipHitShips_++] = shipHitShip;
  }
}

void SensorHandler::handleShipHitWall(
    Ship *hittingShip, double bounceAngle, double bounceForce, int time) {
  int teamIndex = hittingShip->teamIndex;
  if (numHitWalls_[teamIndex] < MAX_HIT_WALLS) {
    ShipHitWall *hitWall = new ShipHitWall;
    hitWall->time = time;
    hitWall->shipIndex = hittingShip->index;
    hitWall->shipX = hittingShip->x;
    hitWall->shipY = hittingShip->y;
    hitWall->bounceAngle = bounceAngle;
    hitWall->bounceForce = bounceForce;
    hitWalls_[teamIndex][numHitWalls_[teamIndex]++] = hitWall;
  }

  if (numStageShipHitWalls_ < MAX_SHIP_HIT_WALLS) {
    ShipHitWall *hitWall = new ShipHitWall;
    hitWall->time = time;
    hitWall->shipIndex = hittingShip->index;
    hitWall->shipX = hittingShip->x;
    hitWall->shipY = hittingShip->y;
    hitWall->bounceAngle = bounceAngle;
    hitWall->bounceForce = bounceForce;
    stageShipHitWalls_[numStageShipHitWalls_++] = hitWall;
  }
}

void SensorHandler::handleShipDestroyed(Ship *destroyedShip, int time,
    Ship **destroyerShips, int numDestroyers) {
  int destroyedShipIndex = destroyedShip->index;
  for (int x = 0; x < numTeams_; x++) {
    if (numShipDestroyeds_[x] < MAX_SHIP_DESTROYEDS) {
      ShipDestroyed *shipDestroyed = new ShipDestroyed;
      shipDestroyed->time = time;
      shipDestroyed->shipIndex = destroyedShipIndex;
      shipDestroyeds_[x][numShipDestroyeds_[x]++] = shipDestroyed;
    }
  }

  if (numStageShipDestroyeds_ < MAX_SHIP_DESTROYEDS) {
    ShipDestroyed *shipDestroyed = new ShipDestroyed;
    shipDestroyed->time = time;
    shipDestroyed->shipIndex = destroyedShipIndex;
    stageShipDestroyeds_[numStageShipDestroyeds_++] = shipDestroyed;
  }
}

void SensorHandler::handleShipFiredLaser(
    Ship *firingShip, double laserHeading, int time) {
  if (numStageShipFiredLasers_ < MAX_SHIP_FIRED_LASERS) {
    StageShipFiredLaser *shipFiredLaser = new StageShipFiredLaser;
    shipFiredLaser->time = time;
    shipFiredLaser->shipIndex = firingShip->index;
    shipFiredLaser->shipX = firingShip->x;
    shipFiredLaser->shipY = firingShip->y;
    shipFiredLaser->laserHeading = laserHeading;
    stageShipFiredLasers_[numStageShipFiredLasers_++] = shipFiredLaser;
  }

  int targetShipIndex = firingShip->index;
  int teamIndex = firingShip->teamIndex;
  for (int x = 0; x < numTeams_; x++) {
    if (x != teamIndex && teamVision_[x][targetShipIndex]) {
      if (numShipFiredLasers_[x] < MAX_SHIP_FIRED_LASERS) {
        ShipFiredLaser *shipFiredLaser = new ShipFiredLaser;
        shipFiredLaser->time = time;
        shipFiredLaser->shipIndex = targetShipIndex;
        shipFiredLaser->shipX = firingShip->x;
        shipFiredLaser->shipY = firingShip->y;
        shipFiredLasers_[x][numShipFiredLasers_[x]++] = shipFiredLaser;
      }
    }
  }
}

void SensorHandler::handleShipFiredTorpedo(
    Ship *firingShip, double torpedoHeading, double torpedoDistance, int time) {
  if (numStageShipFiredTorpedos_ < MAX_SHIP_FIRED_TORPEDOS) {
    StageShipFiredTorpedo *shipFiredTorpedo = new StageShipFiredTorpedo;
    shipFiredTorpedo->time = time;
    shipFiredTorpedo->shipIndex = firingShip->index;
    shipFiredTorpedo->shipX = firingShip->x;
    shipFiredTorpedo->shipY = firingShip->y;
    shipFiredTorpedo->torpedoHeading = torpedoHeading;
    shipFiredTorpedo->torpedoDistance = torpedoDistance;
    stageShipFiredTorpedos_[numStageShipFiredTorpedos_++] = shipFiredTorpedo;
  }

  int firingShipIndex = firingShip->index;
  int teamIndex = firingShip->teamIndex;
  for (int x = 0; x < numTeams_; x++) {
    if (x != teamIndex && teamVision_[x][firingShipIndex]) {
      if (numShipFiredTorpedos_[x] < MAX_SHIP_FIRED_TORPEDOS) {
        ShipFiredTorpedo *shipFiredTorpedo = new ShipFiredTorpedo;
        shipFiredTorpedo->time = time;
        shipFiredTorpedo->shipIndex = firingShipIndex;
        shipFiredTorpedo->shipX = firingShip->x;
        shipFiredTorpedo->shipY = firingShip->y;
        shipFiredTorpedos_[x][numShipFiredTorpedos_[x]++] = shipFiredTorpedo;
      }
    }
  }
}

HitByShip** SensorHandler::getHitByShips(int teamIndex) {
  return hitByShips_[teamIndex];
}

int SensorHandler::numHitByShips(int teamIndex) {
  return numHitByShips_[teamIndex];
}

void SensorHandler::clearHitByShips(int teamIndex) {
  for (int x = 0; x < numHitByShips_[teamIndex]; x++) {
    delete hitByShips_[teamIndex][x];
  }
  numHitByShips_[teamIndex] = 0;
}

HitByLaser** SensorHandler::getHitByLasers(int teamIndex) {
  return hitByLasers_[teamIndex];
}

int SensorHandler::numHitByLasers(int teamIndex) {
  return numHitByLasers_[teamIndex];
}

void SensorHandler::clearHitByLasers(int teamIndex) {
  for (int x = 0; x < numHitByLasers_[teamIndex]; x++) {
    delete hitByLasers_[teamIndex][x];
  }
  numHitByLasers_[teamIndex] = 0;
}

HitByTorpedo** SensorHandler::getHitByTorpedos(int teamIndex) {
  return hitByTorpedos_[teamIndex];
}

int SensorHandler::numHitByTorpedos(int teamIndex) {
  return numHitByTorpedos_[teamIndex];
}

void SensorHandler::clearHitByTorpedos(int teamIndex) {
  for (int x = 0; x < numHitByTorpedos_[teamIndex]; x++) {
    delete hitByTorpedos_[teamIndex][x];
  }
  numHitByTorpedos_[teamIndex] = 0;
}

ShipHitWall** SensorHandler::getHitWalls(int teamIndex) {
  return hitWalls_[teamIndex];
}

int SensorHandler::numHitWalls(int teamIndex) {
  return numHitWalls_[teamIndex];
}

void SensorHandler::clearHitWalls(int teamIndex) {
  for (int x = 0; x < numHitWalls_[teamIndex]; x++) {
    delete hitWalls_[teamIndex][x];
  }
  numHitWalls_[teamIndex] = 0;
}

ShipDestroyed** SensorHandler::getShipDestroyeds(int teamIndex) {
  return shipDestroyeds_[teamIndex];
}

int SensorHandler::numShipDestroyeds(int teamIndex) {
  return numShipDestroyeds_[teamIndex];
}

void SensorHandler::clearShipDestroyeds(int teamIndex) {
  for (int x = 0; x < numShipDestroyeds_[teamIndex]; x++) {
    delete shipDestroyeds_[teamIndex][x];
  }
  numShipDestroyeds_[teamIndex] = 0;
}

ShipFiredLaser** SensorHandler::getShipFiredLasers(int teamIndex) {
  return shipFiredLasers_[teamIndex];
}

int SensorHandler::numShipFiredLasers(int teamIndex) {
  return numShipFiredLasers_[teamIndex];
}

void SensorHandler::clearShipFiredLasers(int teamIndex) {
  for (int x = 0; x < numShipFiredLasers_[teamIndex]; x++) {
    delete shipFiredLasers_[teamIndex][x];
  }
  numShipFiredLasers_[teamIndex] = 0;
}

ShipFiredTorpedo** SensorHandler::getShipFiredTorpedos(int teamIndex) {
  return shipFiredTorpedos_[teamIndex];
}

int SensorHandler::numShipFiredTorpedos(int teamIndex) {
  return numShipFiredTorpedos_[teamIndex];
}

void SensorHandler::clearShipFiredTorpedos(int teamIndex) {
  for (int x = 0; x < numShipFiredTorpedos_[teamIndex]; x++) {
    delete shipFiredTorpedos_[teamIndex][x];
  }
  numShipFiredTorpedos_[teamIndex] = 0;
}

LaserHitShip** SensorHandler::getLaserHitShips(int teamIndex) {
  return laserHitShips_[teamIndex];
}

int SensorHandler::numLaserHitShips(int teamIndex) {
  return numLaserHitShips_[teamIndex];
}

void SensorHandler::clearLaserHitShips(int teamIndex) {
  for (int x = 0; x < numLaserHitShips_[teamIndex]; x++) {
    delete laserHitShips_[teamIndex][x];
  }
  numLaserHitShips_[teamIndex] = 0;
}

ShipHitShip** SensorHandler::getStageShipHitShips() {
  return stageShipHitShips_;
}

int SensorHandler::numStageShipHitShips() {
  return numStageShipHitShips_;
}

void SensorHandler::clearStageShipHitShips() {
  for (int x = 0; x < numStageShipHitShips_; x++) {
    delete stageShipHitShips_[x];
  }
  numStageShipHitShips_ = 0;
}

StageLaserHitShip** SensorHandler::getStageLaserHitShips() {
  return stageLaserHitShips_;
}

int SensorHandler::numStageLaserHitShips() {
  return numStageLaserHitShips_;
}

void SensorHandler::clearStageLaserHitShips() {
  for (int x = 0; x < numStageLaserHitShips_; x++) {
    delete stageLaserHitShips_[x];
  }
  numStageLaserHitShips_ = 0;
}

StageTorpedoHitShip** SensorHandler::getStageTorpedoHitShips() {
  return stageTorpedoHitShips_;
}

int SensorHandler::numStageTorpedoHitShips() {
  return numStageTorpedoHitShips_;
}

void SensorHandler::clearStageTorpedoHitShips() {
  for (int x = 0; x < numStageTorpedoHitShips_; x++) {
    delete stageTorpedoHitShips_[x];
  }
  numStageTorpedoHitShips_ = 0;
}

ShipHitWall** SensorHandler::getStageShipHitWalls() {
  return stageShipHitWalls_;
}

int SensorHandler::numStageShipHitWalls() {
  return numStageShipHitWalls_;
}

void SensorHandler::clearStageShipHitWalls() {
  for (int x = 0; x < numStageShipHitWalls_; x++) {
    delete stageShipHitWalls_[x];
  }
  numStageShipHitWalls_ = 0;
}

ShipDestroyed** SensorHandler::getStageShipDestroyeds() {
  return stageShipDestroyeds_;
}

int SensorHandler::numStageShipDestroyeds() {
  return numStageShipDestroyeds_;
}

void SensorHandler::clearStageShipDestroyeds() {
  for (int x = 0; x < numStageShipDestroyeds_; x++) {
    delete stageShipDestroyeds_[x];
  }
  numStageShipDestroyeds_ = 0;
}

StageShipFiredLaser** SensorHandler::getStageShipFiredLasers() {
  return stageShipFiredLasers_;
}

int SensorHandler::numStageShipFiredLasers() {
  return numStageShipFiredLasers_;
}

void SensorHandler::clearStageShipFiredLasers() {
  for (int x = 0; x < numStageShipFiredLasers_; x++) {
    delete stageShipFiredLasers_[x];
  }
  numStageShipFiredLasers_ = 0;
}

StageShipFiredTorpedo** SensorHandler::getStageShipFiredTorpedos() {
  return stageShipFiredTorpedos_;
}

int SensorHandler::numStageShipFiredTorpedos() {
  return numStageShipFiredTorpedos_;
}

void SensorHandler::clearStageShipFiredTorpedos() {
  for (int x = 0; x < numStageShipFiredTorpedos_; x++) {
    delete stageShipFiredTorpedos_[x];
  }
  numStageShipFiredTorpedos_ = 0;
}

SensorHandler::~SensorHandler() {
  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numHitByShips_[x]; y++) {
      delete hitByShips_[x][y];
    }
    delete hitByShips_[x];
  }
  delete hitByShips_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numHitByLasers_[x]; y++) {
      delete hitByLasers_[x][y];
    }
    delete hitByLasers_[x];
  }
  delete hitByLasers_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numHitByTorpedos_[x]; y++) {
      delete hitByTorpedos_[x][y];
    }
    delete hitByTorpedos_[x];
  }
  delete hitByTorpedos_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numHitWalls_[x]; y++) {
      delete hitWalls_[x][y];
    }
    delete hitWalls_[x];
  }
  delete hitWalls_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numShipDestroyeds_[x]; y++) {
      delete shipDestroyeds_[x][y];
    }
    delete shipDestroyeds_[x];
  }
  delete shipDestroyeds_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numShipFiredLasers_[x]; y++) {
      delete shipFiredLasers_[x][y];
    }
    delete shipFiredLasers_[x];
  }
  delete shipFiredLasers_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numShipFiredTorpedos_[x]; y++) {
      delete shipFiredTorpedos_[x][y];
    }
    delete shipFiredTorpedos_[x];
  }
  delete shipFiredTorpedos_;

  for (int x = 0; x < numTeams_; x++) {
    for (int y = 0; y < numLaserHitShips_[x]; y++) {
      delete laserHitShips_[x][y];
    }
    delete laserHitShips_[x];
  }
  delete laserHitShips_;

  for (int x = 0; x < numStageShipHitShips_; x++) {
    delete stageShipHitShips_[x];
  }

  for (int x = 0; x < numStageLaserHitShips_; x++) {
    delete stageLaserHitShips_[x];
  }

  for (int x = 0; x < numStageTorpedoHitShips_; x++) {
    delete stageTorpedoHitShips_[x];
  }

  for (int x = 0; x < numStageShipHitWalls_; x++) {
    delete stageShipHitWalls_[x];
  }

  for (int x = 0; x < numStageShipFiredLasers_; x++) {
    delete stageShipFiredLasers_[x];
  }

  for (int x = 0; x < numStageShipFiredTorpedos_; x++) {
    delete stageShipFiredTorpedos_[x];
  }

  delete numHitByShips_;
  delete numHitByLasers_;
  delete numHitByTorpedos_;
  delete numHitWalls_;
  delete numShipDestroyeds_;
  delete numShipFiredLasers_;
  delete numShipFiredTorpedos_;
  delete numLaserHitShips_;
}
