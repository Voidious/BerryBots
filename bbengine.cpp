/*
  Copyright (C) 2012 - Voidious

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

#include <stdio.h>
#include <string.h>
#include <platformstl/performance/performance_counter.hpp>
#include "bbconst.h"
#include "bblua.h"
#include "bbengine.h"

BerryBotsEngine::BerryBotsEngine() {
  stage_ = new Stage(DEFAULT_STAGE_WIDTH, DEFAULT_STAGE_HEIGHT);
  gameTime_ = 0;
  numTeams_ = 0;
  teamSize_ = 1;
  numShips_ = 0;
  stageRun_ = false;
  configureComplete_ = false;
  initComplete_ = false;
  battleMode_ = false;
  roundOver_ = false;
  gameOver_ = false;
  cpuTimeSlot_ = 0;
  winnerName_[0] = '\0';

  stageState_ = 0;
  stageDir_ = 0;
  stageFilename_ = 0;
  teams_ = 0;
  ships_ = 0;
  stageShips_ = 0;
  oldShips_ = 0;
  shipProperties_ = 0;
  worlds_ = 0;
  teamVision_ = 0;
  sensorHandler_ = 0;
}

Stage* BerryBotsEngine::getStage() {
  return stage_;
}

Ship** BerryBotsEngine::getShips() {
  return ships_;
}

int BerryBotsEngine::getNumShips() {
  return numShips_;
}

int BerryBotsEngine::getGameTime() {
  return gameTime_;
}

void BerryBotsEngine::setTeamSize(int teamSize) {
  teamSize_ = teamSize;
}

int BerryBotsEngine::getTeamSize() {
  return teamSize_;
}

bool BerryBotsEngine::isConfigureComplete() {
  return configureComplete_;
}

bool BerryBotsEngine::isInitComplete() {
  return initComplete_;
}

void BerryBotsEngine::setBattleMode(bool battleMode) {
  battleMode_ = battleMode;
}

bool BerryBotsEngine::isBattleMode() {
  return battleMode_;
}

void BerryBotsEngine::setRoundOver(bool roundOver) {
  roundOver_ = roundOver;
}

bool BerryBotsEngine::isRoundOver() {
  return roundOver_;
}

void BerryBotsEngine::setGameOver(bool gameOver) {
  gameOver_ = gameOver;
}

bool BerryBotsEngine::isGameOver() {
  return gameOver_;
}

void BerryBotsEngine::setWinnerName(const char* winnerName) {
  strcpy(winnerName_, winnerName);
}

char* BerryBotsEngine::getWinnerName() {
  if (strlen(winnerName_) == 0) {
    return 0;
  } else {
    char* newWinnerName = new char[strlen(winnerName_) + 1];
    strcpy(newWinnerName, winnerName_);
    return newWinnerName;
  }
}

Team** BerryBotsEngine::getTeams() {
  return teams_;
}

Team* BerryBotsEngine::getTeam(int teamIndex) {
  return teams_[teamIndex];
}

Team* BerryBotsEngine::getTeam(lua_State *L) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->state == L) {
      return team;
    }
  }
  return 0;
}

int BerryBotsEngine::getNumTeams() {
  return numTeams_;
}

void BerryBotsEngine::initStage(char *stagePath, const char *cacheDir) {
  char *stageCwd;
  loadStageFile(stagePath, &stageDir_, &stageFilename_, &stageCwd, cacheDir);
  initStageState(&stageState_, stageCwd, stageFilename_);  

  if (luaL_loadfile(stageState_, stageFilename_)
      || lua_pcall(stageState_, 0, 0, 0)) {
    luaL_error(stageState_, "cannot load stage file: %s",
               lua_tostring(stageState_, -1));
  }

  lua_getglobal(stageState_, "configure");
  pushStageBuilder(stageState_);
  if (lua_pcall(stageState_, 1, 0, 0) != 0) {
    luaL_error(stageState_, "error calling stage function: 'configure': %s",
               lua_tostring(stageState_, -1));
  }
  stage_->buildBaseWalls();
  stage_->setName(stageFilename_); // TODO: let stage set name like ship
  configureComplete_ = true;
  delete stageCwd;
}

void BerryBotsEngine::initShips(
    char **teamPaths, int numTeams, const char *cacheDir) {
  int userTeams = numTeams;
  int numStageShips = stage_->getStageShipCount();
  numShips_ = (userTeams * teamSize_) + numStageShips;
  numTeams_ = userTeams + numStageShips;
  ships_ = new Ship*[numShips_];
  shipProperties_ = new ShipProperties*[numShips_];
  teams_ = new Team*[numTeams_];
  worlds_ = new World*[numTeams_];
  int shipIndex = 0;
  for (int x = 0; x < numTeams_; x++) {
    char *filename;
    bool deleteFilename = false;
    bool stageShip = (x >= userTeams);
    if (stageShip) {
      filename = stage_->getStageShips()[x - userTeams];
      if (stageDir_ != 0) {
        char *rawFilename = filename;
        filename = new char[strlen(stageDir_) + strlen(rawFilename) + 2];
        sprintf(filename, "%s/%s", stageDir_, rawFilename);
        deleteFilename = true;
      }
    } else {
      filename = teamPaths[x];
    }

    lua_State *teamState;
    char *shipDir;
    char *shipFilename;
    char *shipCwd;
    loadBotFile(filename, &shipDir, &shipFilename, &shipCwd, cacheDir);
    initShipState(&teamState, shipCwd, shipFilename);  

    int numStateShips = (stageShip ? 1 : teamSize_);
    Ship **stateShips = new Ship*[numStateShips];
    bool doa;
    if (luaL_loadfile(teamState, shipFilename)
        || lua_pcall(teamState, 0, 0, 0)) {
      printf("cannot load file: %s\n", lua_tostring(teamState, -1));
      doa = true;
    } else {
      lua_getglobal(teamState, "init");
      doa = false;
    }

    if (!doa && numStateShips > 1) {
      // If it's a team, pass a table of ships instead of a single ship.
      lua_newtable(teamState);
    }

    Team *team = new Team;
    team->index = x;
    team->firstShipIndex = shipIndex;
    team->numShips = numStateShips;
    team->state = teamState;
    team->shipsAlive = 0;
    team->stageEventRef = 0;
    for (int y = 0; y < CPU_TIME_TICKS; y++) {
      team->cpuTime[y] = 0;
    }
    team->totalCpuTime = 0;
    team->totalCpuTicks = 0;
    team->doa = doa;

    lua_getglobal(teamState, "roundOver");
    team->hasRoundOver = (strcmp(luaL_typename(teamState, -1), "nil") != 0);
    lua_getglobal(teamState, "gameOver");
    team->hasGameOver = (strcmp(luaL_typename(teamState, -1), "nil") != 0);
    lua_pop(teamState, 2);

    char *extension = strrchr(shipFilename, '.');
    int nameLength = min(MAX_NAME_LENGTH,
        (extension == 0) ? strlen(shipFilename) : extension - shipFilename);
    strncpy(team->name, shipFilename, nameLength);
    team->name[nameLength] = '\0';
    team->stageShip = stageShip;

    for (int y = 0; y < numStateShips; y++) {
      Ship *ship;
      if (doa) {
        ship = new Ship;
        ship->properties = new ShipProperties;
        ship->properties->doa = true;
        ship->alive = false;
      } else {
        ship = pushShip(teamState);
        if (numStateShips > 1) {
          lua_rawseti(teamState, -2, y + 1);
        }
        ship->properties->doa = false;
        ship->alive = true;
      }
      ship->teamIndex = x;
      ship->index = shipIndex++;
      ships_[ship->index] = stateShips[y] = ship;
    }

    for (int y = 0; y < numStateShips; y++) {
      Ship *ship = stateShips[y];
      ShipProperties *properties = ship->properties;
      ship->laserEnabled = battleMode_;
      ship->torpedoEnabled = battleMode_;
      ship->thrusterEnabled = true;
      ship->energyEnabled = battleMode_;
      ship->showName = true;
      ship->kills = ship->damage = 0;
      ship->friendlyKills = ship->friendlyDamage = 0;
      properties->stageShip = stageShip;
      properties->shipR = properties->shipG = properties->shipB = 255;
      properties->laserR = properties->laserB = 0;
      properties->laserG = 255;
      properties->thrusterG = properties->thrusterB = 0;
      properties->thrusterR = 255;
  
      strncpy(properties->name, shipFilename, nameLength);
      properties->name[nameLength] = '\0';
      initShipRound(ship);
    }

    teams_[x] = team;
    for (int y = 0; y < numStateShips; y++) {
      Ship *ship = stateShips[y];
      ships_[ship->index] = ship;
    }

    if (!doa) {
      worlds_[x] = pushWorld(teamState, stage_, numShips_, teamSize_);
      if (lua_pcall(teamState, 2, 0, 0) != 0) {
        printf("error calling ship (%s) function: 'init': %s\n",
               shipFilename, lua_tostring(teamState, -1));
        for (int y = 0; y < numStateShips; y++) {
          stateShips[y]->alive = false;
        }
      }
    }

    if (deleteFilename) {
      delete filename;
    }
    if (shipDir != 0) {
      delete shipDir;
    }
    delete shipFilename;
    delete stateShips;
  }

  uniqueShipNames(ships_, numShips_);
  uniqueTeamNames(teams_, numTeams_);
  teamVision_ = new bool*[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    teamVision_[x] = new bool[numShips_];
  }
  sensorHandler_ = new SensorHandler(teams_, numTeams_, teamVision_);
  stage_->addEventHandler((EventHandler*) sensorHandler_);

  lua_getglobal(stageState_, "init");
  stageShips_ = new Ship*[numShips_];
  pushStageShips(stageState_, ships_, stageShips_, numShips_);
  stage_->setTeamsAndShips(teams_, numTeams_, stageShips_, numShips_);
  if (strcmp(luaL_typename(stageState_, -2), "nil") != 0) {
    stageWorld_ = pushWorld(stageState_, stage_, numShips_, teamSize_);
    pushAdmin(stageState_);
    if (lua_pcall(stageState_, 3, 0, 0) != 0) {
      luaL_error(stageState_, "error calling stage function: 'init': %s",
                 lua_tostring(stageState_, -1));
    }
  }

  copyShips(stageShips_, ships_, numShips_);
  oldShips_ = new Ship*[numShips_];
  for (int x = 0; x < numShips_; x++) {
    oldShips_[x] = new Ship;
  }

  lua_getglobal(stageState_, "run");
  stageRun_ = (strcmp(luaL_typename(stageState_, -1), "nil") != 0);
  lua_settop(stageState_, 0);

  for (int x = 0; x < numShips_; x++) {
    shipProperties_[x] = ships_[x]->properties;
  }

  initComplete_ = true;
}

void BerryBotsEngine::initShipRound(Ship *ship) {
  if (!ship->properties->doa) {
    ship->alive = true;
  }
  ship->speed = ship->heading = ship->thrusterAngle = ship->thrusterForce = 0;
  ship->energy = DEFAULT_ENERGY;
  ship->laserGunHeat = LASER_HEAT;
  ship->torpedoGunHeat = TORPEDO_HEAT;
  ship->hitWall = ship->hitShip = false;

  bool safeStart;
  do {
    Point2D *startPosition = stage_->getStart();
    ship->x = startPosition->getX();
    ship->y = startPosition->getY();
    safeStart = true;
    for (int y = 0; y < ship->index; y++) {
      if (ships_[y]->alive
          && square(ship->x - ships_[y]->x) + square(ship->y - ships_[y]->y)
                < square(SHIP_RADIUS * 2)) {
        safeStart = false;
      }
    }
    delete startPosition;
  } while (!safeStart);
}

void BerryBotsEngine::updateTeamShipsAlive() {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    int shipsAlive = 0;
    for (int y = 0; y < team->numShips; y++) {
      Ship *ship = ships_[y + team->firstShipIndex];
      if (ship->alive) {
        shipsAlive++;
      }
    }
    team->shipsAlive = shipsAlive;
  }
}

void BerryBotsEngine::processTick() {
  cpuTimeSlot_ = gameTime_ % CPU_TIME_TICKS;
  gameTime_++;
  updateTeamShipsAlive();    
  stage_->updateTeamVision(teams_, numTeams_, ships_, numShips_, teamVision_);
  copyShips(ships_, oldShips_, numShips_);
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->shipsAlive > 0) {
      worlds_[x]->time = gameTime_;
      for (int y = 0; y < team->numShips; y++) {
        int shipIndex = y + team->firstShipIndex;
        Ship *ship = ships_[shipIndex];
        ship->thrusterForce = 0;
        ship->laserGunHeat = max(0, ship->laserGunHeat - 1);
        ship->torpedoGunHeat = max(0, ship->torpedoGunHeat - 1);
      }

      lua_getglobal(team->state, "run");
      pushVisibleEnemyShips(
          team->state, teamVision_[x], x, oldShips_, numShips_);
      Sensors *sensors = pushSensors(team, sensorHandler_, shipProperties_);
      team->counter.start();
      if (lua_pcall(team->state, 2, 0, 0) != 0) {
        printf("error calling ship (%s) function: 'run': %s\n",
               team->name, lua_tostring(team->state, -1));
      }
      team->counter.stop();
      team->totalCpuTime += team->cpuTime[cpuTimeSlot_] =
          team->counter.get_microseconds();
      team->totalCpuTicks++;
      cleanupSensorsTables(team->state, sensors);
      lua_settop(team->state, 0);
    }
  }
  stage_->moveAndCheckCollisions(oldShips_, ships_, numShips_, gameTime_);
  
  if (stageRun_) {
    this->setRoundOver(false);
    this->setGameOver(false);
    processStageRun();
  }
}

void BerryBotsEngine::processStageRun() {
  copyShips(ships_, stageShips_, numShips_);
  stageWorld_->time = gameTime_;
  lua_getglobal(stageState_, "run");
  StageSensors *stageSensors = pushStageSensors(
      stageState_, sensorHandler_, stageShips_, shipProperties_);
  if (lua_pcall(stageState_, 1, 0, 0) != 0) {
    luaL_error(stageState_, "error calling stage function: 'run': %s",
               lua_tostring(stageState_, -1));
  }
  cleanupStageSensorsTables(stageState_, stageSensors);
  lua_settop(stageState_, 0);
  copyShips(stageShips_, ships_, numShips_);
}

void BerryBotsEngine::processRoundOver() {
  copyShips(stageShips_, ships_, numShips_);
  stage_->reset();
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->hasRoundOver) {
      lua_getglobal(team->state, "roundOver");
      team->counter.start();
      if (lua_pcall(team->state, 0, 0, 0) != 0) {
        printf("error calling ship (%s) function: 'roundOver': %s\n",
               team->name, lua_tostring(team->state, -1));
      }
      team->counter.stop();
      team->totalCpuTime += team->cpuTime[cpuTimeSlot_] =
          team->counter.get_microseconds();
    }
    for (int y = 0; y < team->numShips; y++) {
      Ship *ship = ships_[team->firstShipIndex + y];
      if (!ship->properties->stageShip) {
        initShipRound(ships_[team->firstShipIndex + y]);
      }
    }
  }
  copyShips(ships_, stageShips_, numShips_);
}

void BerryBotsEngine::processGameOver() {
  copyShips(stageShips_, ships_, numShips_);
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->hasGameOver) {
      lua_getglobal(team->state, "gameOver");
      team->counter.start();
      if (lua_pcall(team->state, 0, 0, 0) != 0) {
        printf("error calling ship (%s) function: 'gameOver': %s\n",
               team->name, lua_tostring(team->state, -1));
      }
      team->counter.stop();
      team->totalCpuTime += team->cpuTime[cpuTimeSlot_] =
          team->counter.get_microseconds();
    }
  }
  copyShips(ships_, stageShips_, numShips_);
}

bool BerryBotsEngine::touchedZone(Ship *ship, const char *zoneTag) {
  return stage_->touchedZone(oldShips_[ship->index], ship, zoneTag);
}

bool BerryBotsEngine::touchedAnyZone(Ship *ship) {
  return stage_->touchedAnyZone(oldShips_[ship->index], ship);
}

void BerryBotsEngine::destroyShip(Ship *ship) {
  stage_->destroyShip(ship, gameTime_);
}

void BerryBotsEngine::uniqueShipNames(Ship** ships, int numShips) {
  for (int x = 1; x < numShips; x++) {
    Ship *ship1 = ships[x];
    int nameNum = 1;
    int nameLen = strlen(ship1->properties->name);
    char numStr[8];
    for (int y = 0; y < x; y++) {
      Ship *ship2 = ships[y];
      if (strcmp(ship1->properties->name, ship2->properties->name) == 0) {
        sprintf(numStr, " %d", ++nameNum);
        strncpy(&(ship1->properties->name[min(nameLen, MAX_NAME_LENGTH - 7)]),
                numStr, 7);
        y = -1;
      }
    }
  }
}

void BerryBotsEngine::uniqueTeamNames(Team** teams, int numTeams) {
  for (int x = 1; x < numTeams; x++) {
    Team *team1 = teams[x];
    int nameNum = 1;
    int nameLen = strlen(team1->name);
    char numStr[8];
    for (int y = 0; y < x; y++) {
      Team *team2 = teams[y];
      if (strcmp(team1->name, team2->name) == 0) {
        sprintf(numStr, " %d", ++nameNum);
        strncpy(&(team1->name[min(nameLen, MAX_NAME_LENGTH - 7)]), numStr, 7);
        y = -1;
      }
    }
  }
}

void BerryBotsEngine::copyShips(
    Ship **srcShips, Ship **destShips, int numShips) {
  for (int x = 0; x < numShips; x++) {
    *(destShips[x]) = *(srcShips[x]);
  }
}

BerryBotsEngine::~BerryBotsEngine() {
  if (stageDir_ != 0) {
    delete stageDir_;
  }
  if (stageFilename_ != 0) {
    delete stageFilename_;
  }

  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships_[x];
    if (ship->properties->doa) {
      delete ship->properties;
      delete ship;
    } else {
      delete ship->properties;
    }
    delete oldShips_[x];
  }
  delete ships_;
  delete oldShips_;

  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (!team->doa) {
      lua_close(teams_[x]->state);
    }
    delete teams_[x];
  }
  delete teams_;
  lua_close(stageState_);

  delete worlds_;
  for (int x = 0; x < numTeams_; x++) {
    delete teamVision_[x];
  }
  delete teamVision_;
  delete stage_;
}
