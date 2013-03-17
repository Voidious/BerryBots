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

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <pthread.h>
#include <limits.h>
#include <platformstl/performance/performance_counter.hpp>
#include <platformstl/synch/sleep_functions.h>
#include "bbconst.h"
#include "bblua.h"
#include "printhandler.h"
#include "zipper.h"
#include "bbengine.h"

extern PrintHandler *printHandler;

BerryBotsEngine::BerryBotsEngine(FileManager *fileManager) {
  stage_ = new Stage(DEFAULT_STAGE_WIDTH, DEFAULT_STAGE_HEIGHT);
  listener_ = 0;
  gameTime_ = 0;
  numTeams_ = numInitializedTeams_ = 0;
  teamSize_ = 1;
  numShips_ = numInitializedShips_ = 0;
  stageRun_ = false;
  stageConfigureComplete_ = false;
  shipInitComplete_ = false;
  battleMode_ = false;
  roundOver_ = false;
  gameOver_ = false;
  winnerName_[0] = '\0';

  stageState_ = 0;
  stagesDir_ = 0;
  stageFilename_ = 0;
  stageWorld_ = 0;
  teams_ = 0;
  ships_ = 0;
  stageShips_ = 0;
  oldShips_ = 0;
  shipProperties_ = 0;
  worlds_ = 0;
  shipGfxs_ = 0;
  teamVision_ = 0;
  sensorHandler_ = 0;
  fileManager_ = fileManager;

  timerSettings_ = new TimerSettings;
  timerSettings_->L = 0;
  timerSettings_->timerTick = 0;
  timerSettings_->timerExpiration = LONG_MAX;
  timerSettings_->enabled = true;
  pthread_create(&timerThread_, 0, BerryBotsEngine::timerThread,
                (void*) timerSettings_);
  // TODO: how to handle failure to create thread
}

BerryBotsEngine::~BerryBotsEngine() {
  timerSettings_->enabled = false;
  if (stagesDir_ != 0) {
    delete stagesDir_;
  }
  if (stageFilename_ != 0) {
    delete stageFilename_;
  }

  for (int x = 0; x < numInitializedShips_; x++) {
    Ship *ship = ships_[x];
    if (ship->properties->ownedByLua) {
      delete ship->properties;
    } else {
      delete ship->properties;
      delete ship;
    }
  }
  delete ships_;
  delete shipProperties_;
  if (oldShips_ != 0) {
    for (int x = 0; x < numShips_; x++) {
      delete oldShips_[x];
    }
    delete oldShips_;
  }

  for (int x = 0; x < numInitializedTeams_; x++) {
    Team *team = teams_[x];
    if (team->ownedByLua) {
      lua_close(team->state);
    }
    for (int y = 0; y < team->numRectangles; y++) {
      delete team->shipGfxRectangles[y];
    }
    for (int y = 0; y < team->numLines; y++) {
      delete team->shipGfxLines[y];
    }
    for (int y = 0; y < team->numCircles; y++) {
      delete team->shipGfxCircles[y];
    }
    for (int y = 0; y < team->numTexts; y++) {
      delete team->shipGfxTexts[y];
    }
    delete team;
  }
  delete teams_;
  if (stageState_ != 0) {
    lua_close(stageState_);
  }

  if (worlds_ != 0) {
    delete worlds_;
  }
  if (shipGfxs_ != 0) {
    delete shipGfxs_;
  }
  if (teamVision_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      delete teamVision_[x];
    }
    delete teamVision_;
  }
  delete stage_;
  if (sensorHandler_ != 0) {
    delete sensorHandler_;
  }
}

void BerryBotsEngine::setListener(NewTeamStateListener *listener) {
  listener_ = listener;
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

bool BerryBotsEngine::isStageConfigureComplete() {
  return stageConfigureComplete_;
}

bool BerryBotsEngine::isShipInitComplete() {
  return shipInitComplete_;
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
  int winnerNameLen = std::min((int) strlen(winnerName), MAX_NAME_LENGTH);
  strncpy(winnerName_, winnerName, winnerNameLen);
  winnerName_[winnerNameLen] = '\0';
}

const char* BerryBotsEngine::getWinnerName() {
  if (strlen(winnerName_) == 0) {
    return 0;
  } else {
    return winnerName_;
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

int BerryBotsEngine::callUserLuaCode(lua_State *L, int nargs,
    const char *errorMsg, int callStyle) throw (EngineException*) {
  // TODO: I'm not completely convinced we're safe without a mutex here. Do
  //       more tests and maybe add one.
  timerSettings_->timerExpiration = timerSettings_->timerTick + 2;
  timerSettings_->L = L;
  int pcallValue = lua_pcall(L, nargs, 0, 0);
  timerSettings_->L = 0;
  timerSettings_->timerExpiration = LONG_MAX;

  if (pcallValue != 0) {
    std::string errorString(errorMsg);
    errorString.append(": %s");
    if (callStyle == PCALL_STAGE) {
      throwForLuaError(L, errorString.c_str());
    } else if (callStyle == PCALL_SHIP) {
      printLuaErrorToShipConsole(L, errorString.c_str());
    }
  }

  return pcallValue;
}

void *BerryBotsEngine::timerThread(void *vargs) {
  TimerSettings *settings = (TimerSettings*) vargs;
  while (settings->enabled) {
    platformstl::micro_sleep(PCALL_TIME_LIMIT);
    if (settings->timerExpiration <= ++(settings->timerTick)) {
      if (settings->L != 0) {
        lua_State *killState = settings->L;
        settings->L = 0;
        lua_sethook(killState, killHook, LUA_MASKCOUNT, 1);
      }
    }
  }
  delete settings;
  return 0;
}

// Loads the stage in the file stageName, which may include a relative path,
// from the root directory stagesBaseDir. Note that the file may be either a
// .lua file, in which case we just load it directly; or a stage packaged as a
// .tar.gz file, in which case we extract it to the cache and load from there.
void BerryBotsEngine::initStage(const char *stagesBaseDir,
    const char *stageName, const char *cacheDir) throw (EngineException*) {
  if (stagesDir_ != 0 || stageFilename_ != 0) {
    throw new EngineException(stageName,
                              "Already initialized stage for this engine.");
  }

  try {
    fileManager_->loadStageFileData(
        stagesBaseDir, stageName, &stagesDir_, &stageFilename_, cacheDir);
  } catch (FileNotFoundException *fnfe) {
    EngineException *eie = new EngineException(stageName, fnfe->what());
    delete fnfe;
    throw eie;
  } catch (ZipperException *ze) {
    EngineException *eie = new EngineException(stageName, ze->what());
    delete ze;
    throw eie;
  } catch (PackagedSymlinkException *pse) {
    EngineException *eie = new EngineException(stageName, pse->what());
    delete pse;
    throw eie;
  }
  initStageState(&stageState_, stagesDir_);

  if (luaL_loadfile(stageState_, stageFilename_)) {
    throwForLuaError(stageState_, "Cannot load stage file: %s");
  }
  callUserLuaCode(stageState_, 0, "Cannot load stage file", PCALL_STAGE);

  lua_getglobal(stageState_, "configure");
  StageBuilder *stageBuilder = pushStageBuilder(stageState_);
  stageBuilder->engine = this;
  callUserLuaCode(stageState_, 1, "Error calling stage function: 'configure'",
                  PCALL_STAGE);

  stage_->buildBaseWalls();
  char *stageRootName = FileManager::stripExtension(stageFilename_);
  char *stageDisplayName = FileManager::parseFilename(stageRootName);
  stage_->setName(stageDisplayName); // TODO: let stage set name like ship
  delete stageRootName;
  delete stageDisplayName;
  stageConfigureComplete_ = true;
}

// Loads the teams in the files specified in teamNames from the root directory
// shipsBaseDir. Each team name is a path relative to shipsBaseDir . Note that
// the team file may be either a .lua file, in which case we just load it
// directly; or a ship/team packaged as a .tar.gz file, in which case we extract
// it to the cache and load from there.
void BerryBotsEngine::initShips(const char *shipsBaseDir, char **teamNames,
    int numTeams, const char *cacheDir) throw (EngineException*) {
  int userTeams = numTeams;
  int numStageShips = stage_->getStageShipCount();
  numShips_ = (userTeams * teamSize_) + numStageShips;
  numTeams_ = userTeams + numStageShips;
  ships_ = new Ship*[numShips_];
  shipProperties_ = new ShipProperties*[numShips_];
  teams_ = new Team*[numTeams_];
  worlds_ = new World*[numTeams_];
  shipGfxs_ = new ShipGfx*[numTeams_];
  int shipIndex = 0;
  for (int x = 0; x < numTeams_; x++) {
    const char *baseDir;
    char *filename;
    bool deleteFilename = false;
    bool stageShip = (x >= userTeams);
    if (stageShip) {
      baseDir = stagesDir_;
      const char *localFilename = stage_->getStageShips()[x - userTeams];
      filename = FileManager::getStageShipRelativePath(
          stagesDir_, stageFilename_, localFilename);
      if (filename == 0) {
        throw new EngineException(localFilename,
            "Stage ship must be located under stages directory.");
      }
      deleteFilename = true;
    } else {
      baseDir = shipsBaseDir;
      filename = teamNames[x];
    }

    lua_State *teamState;
    char *shipDir = 0;
    char *shipFilename = 0;
    try {
      fileManager_->loadShipFileData(baseDir, filename, &shipDir, &shipFilename,
                                     cacheDir);
    } catch (FileNotFoundException *fnfe) {
      if (deleteFilename) {
        delete filename;
      }
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename ;
      }

      EngineException *eie = new EngineException(filename, fnfe->what());
      delete fnfe;
      throw eie;
    } catch (ZipperException *ze) {
      if (deleteFilename) {
        delete filename;
      }
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename ;
      }
      
      EngineException *eie = new EngineException(filename, ze->what());
      delete ze;
      throw eie;
    } catch (PackagedSymlinkException *pse) {
      if (deleteFilename) {
        delete filename;
      }
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename ;
      }
      
      EngineException *eie = new EngineException(filename, pse->what());
      delete pse;
      throw eie;
    }
    initShipState(&teamState, shipDir);

    Team *team = new Team;
    team->index = x;
    team->firstShipIndex = shipIndex;
    team->state = teamState;
    team->errored = false;
    team->shipGfxEnabled = false;
    if (listener_ != 0) {
      listener_->newTeam(team, filename);
    }

    int numStateShips = (stageShip ? 1 : teamSize_);
    Ship **stateShips = new Ship*[numStateShips];
    bool disabled;
    if (luaL_loadfile(teamState, shipFilename)) {
      printLuaErrorToShipConsole(teamState, "Error loading file: %s");
      disabled = true;
    } else if (callUserLuaCode(teamState, 0, "Error loading file",
                               PCALL_SHIP)) {
      disabled = true;
    } else {
      lua_getglobal(teamState, "init");
      disabled = false;
    }

    if (!disabled && numStateShips > 1) {
      // If it's a team, pass a table of ships instead of a single ship.
      lua_newtable(teamState);
    }

    team->numShips = numStateShips;
    team->shipsAlive = 0;
    team->stageEventRef = 0;
    for (int y = 0; y < CPU_TIME_TICKS; y++) {
      team->cpuTime[y] = 0;
    }
    team->totalCpuTime = 0;
    team->totalCpuTicks = 0;
    team->disabled = disabled;
    team->numRectangles = 0;
    team->numLines = 0;
    team->numCircles = 0;
    team->numTexts = 0;

    lua_getglobal(teamState, "roundOver");
    team->hasRoundOver = (strcmp(luaL_typename(teamState, -1), "nil") != 0);
    lua_getglobal(teamState, "gameOver");
    team->hasGameOver = (strcmp(luaL_typename(teamState, -1), "nil") != 0);
    lua_pop(teamState, 2);

    char *shipFilenameRoot = FileManager::stripExtension(shipFilename);
    char *defaultShipName = FileManager::parseFilename(shipFilenameRoot);
    int nameLength = (int) strlen(defaultShipName);
    strncpy(team->name, defaultShipName, nameLength);
    team->name[nameLength] = '\0';
    delete shipFilename;
    delete shipFilenameRoot;
    team->stageShip = stageShip;

    for (int y = 0; y < numStateShips; y++) {
      Ship *ship;
      if (disabled) {
        ship = new Ship;
        ship->properties = new ShipProperties;
        ship->properties->disabled = true;
        ship->properties->ownedByLua = false;
      } else {
        ship = pushShip(teamState);
        if (numStateShips > 1) {
          lua_rawseti(teamState, -2, y + 1);
        }
        ship->properties->disabled = false;
        ship->properties->ownedByLua = true;
      }
      ship->alive = false;
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
      properties->engine = this;
  
      strncpy(properties->name, defaultShipName, nameLength);
      properties->name[nameLength] = '\0';
      initShipRound(ship);
    }
    delete defaultShipName;

    teams_[x] = team;
    numInitializedTeams_++;
    for (int y = 0; y < numStateShips; y++) {
      Ship *ship = stateShips[y];
      ships_[ship->index] = ship;
      numInitializedShips_++;
    }

    if (!disabled) {
      worlds_[x] = pushWorld(teamState, stage_, numShips_, teamSize_);
      worlds_[x]->engine = this;
      shipGfxs_[x] = pushShipGfx(teamState);
      shipGfxs_[x]->teamIndex = team->index;
      shipGfxs_[x]->engine = this;

      int r = callUserLuaCode(teamState, 3,
          "Error calling ship function: 'init'", PCALL_SHIP);
      if (r != 0) {
        team->disabled = true;
        for (int y = 0; y < numStateShips; y++) {
          stateShips[y]->alive = false;
          stateShips[y]->properties->disabled = true;
        }
      }
    }

    if (deleteFilename) {
      delete filename;
    }
    if (shipDir != 0) {
      delete shipDir;
    }
    delete stateShips;
  }

  // TODO: print to output console if ship or team name changes
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
  pushCopyOfShips(stageState_, ships_, stageShips_, numShips_);
  stage_->setTeamsAndShips(teams_, numTeams_, stageShips_, numShips_);
  if (strcmp(luaL_typename(stageState_, -2), "nil") != 0) {
    stageWorld_ = pushWorld(stageState_, stage_, numShips_, teamSize_);
    stageWorld_->engine = this;
    Admin *admin = pushAdmin(stageState_);
    admin->engine = this;
    callUserLuaCode(stageState_, 3, "Error calling stage function: 'init'",
                    PCALL_STAGE);
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

  shipInitComplete_ = true;
}

void BerryBotsEngine::initShipRound(Ship *ship) {
  ship->alive = (!ship->properties->disabled && !ship->properties->stageShip);
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

void BerryBotsEngine::processTick() throw (EngineException*) {
  gameTime_++;
  updateTeamShipsAlive();    
  stage_->updateTeamVision(teams_, numTeams_, ships_, numShips_, teamVision_);
  copyShips(ships_, oldShips_, numShips_);
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->shipsAlive > 0 && !team->disabled) {
      worlds_[x]->time = gameTime_;
      for (int y = 0; y < team->numShips; y++) {
        int shipIndex = y + team->firstShipIndex;
        Ship *ship = ships_[shipIndex];
        ship->thrusterForce = 0;
        ship->laserGunHeat = std::max(0, ship->laserGunHeat - 1);
        ship->torpedoGunHeat = std::max(0, ship->torpedoGunHeat - 1);
      }

      lua_getglobal(team->state, "run");
      pushVisibleEnemyShips(
          team->state, teamVision_[x], x, oldShips_, numShips_);
      Sensors *sensors = pushSensors(team, sensorHandler_, shipProperties_);
      team->counter.start();
      int r = callUserLuaCode(team->state, 2,
                              "Error calling ship function: 'run'", PCALL_SHIP);
      monitorCpuTimer(team, (r != 0 && lua_gethookcount(team->state) > 0));
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

void BerryBotsEngine::processStageRun() throw (EngineException*) {
  copyShips(ships_, stageShips_, numShips_);
  if (stageWorld_ != 0) {
    stageWorld_->time = gameTime_;
  }
  lua_getglobal(stageState_, "run");
  StageSensors *stageSensors = pushStageSensors(
      stageState_, sensorHandler_, stageShips_, shipProperties_);
  callUserLuaCode(stageState_, 1, "Error calling stage function: 'run'",
                  PCALL_STAGE);
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
      int r = callUserLuaCode(team->state, 0,
          "Error calling ship function: 'roundOver'", PCALL_SHIP);
      monitorCpuTimer(team, (r != 0 && lua_gethookcount(team->state) > 0));
    }
    for (int y = 0; y < team->numShips; y++) {
      Ship *ship = ships_[team->firstShipIndex + y];
      if (!ship->properties->stageShip && !ship->properties->disabled) {
        initShipRound(ship);
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
      int r = callUserLuaCode(team->state, 0,
          "Error calling ship function: 'gameOver'", PCALL_SHIP);
      monitorCpuTimer(team, (r != 0 && lua_gethookcount(team->state) > 0));
    }
  }
  copyShips(ships_, stageShips_, numShips_);
}

void BerryBotsEngine::monitorCpuTimer(Team *team, bool fatal) {
  unsigned int cpuTimeSlot = team->totalCpuTicks % CPU_TIME_TICKS;
  team->counter.stop();
  team->totalCpuTime +=
  (team->cpuTime[cpuTimeSlot] = team->counter.get_microseconds());
  team->totalCpuTicks++;

  if (fatal) {
    team->disabled = true;
    for (int x = 0; x < team->numShips; x++) {
      Ship *ship = ships_[x + team->firstShipIndex];
      destroyShip(ship);
      ship->properties->disabled = true;
    }
  }
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
    int nameLen = (int) strlen(ship1->properties->name);
    char numStr[8];
    for (int y = 0; y < x; y++) {
      Ship *ship2 = ships[y];
      if (strcmp(ship1->properties->name, ship2->properties->name) == 0) {
        sprintf(numStr, " %d", ++nameNum);
        strncpy(
            &(ship1->properties->name[std::min(nameLen, MAX_NAME_LENGTH - 7)]),
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
    int nameLen = (int) strlen(team1->name);
    char numStr[8];
    for (int y = 0; y < x; y++) {
      Team *team2 = teams[y];
      if (strcmp(team1->name, team2->name) == 0) {
        sprintf(numStr, " %d", ++nameNum);
        strncpy(
            &(team1->name[std::min(nameLen, MAX_NAME_LENGTH - 7)]), numStr, 7);
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

// Note: We don't have to log stage errors to the output console because they
//       are considered fatal. We throw exceptions from the engine and the
//       GUI or CLI displays them appropriately.
void BerryBotsEngine::printLuaErrorToShipConsole(lua_State *L,
                                                 const char *formatString) {
  if (printHandler != 0) {
    char *errorMessage = formatLuaError(L, formatString);
    printHandler->shipPrint(L, errorMessage);
    delete errorMessage;
  }
  for (int x = 0; x < numInitializedTeams_; x++) {
    Team *team = teams_[x];
    if (team->state == L) {
      team->errored = true;
    }
  }
}

void BerryBotsEngine::throwForLuaError(lua_State *L, const char *formatString)
    throw (EngineException*) {
  char *errorMessage = formatLuaError(L, formatString);
  EngineException *e = new EngineException(errorMessage);
  delete errorMessage;
  throw e;
}

char* BerryBotsEngine::formatLuaError(lua_State *L, const char *formatString) {
  const char *luaMessage = lua_tostring(L, -1);
  int messageLen = (int) (strlen(formatString) + strlen(luaMessage) - 2);
  char *errorMessage = new char[messageLen + 1];
  sprintf(errorMessage, formatString, luaMessage);
  return errorMessage;
}

EngineException::EngineException(const char *details) {
  message_ = new char[strlen(details) + 41];
  sprintf(message_, "Engine failure: %s", details);
}

EngineException::EngineException(const char *filename, const char *details) {
  std::string errorMessage(filename);
  errorMessage.append(": Engine failure: ");
  errorMessage.append(details);
  message_ = new char[errorMessage.size() + 1];
  strcpy(message_, errorMessage.c_str());
}

const char* EngineException::what() const throw() {
  return message_;
}

EngineException::~EngineException() throw() {
  delete message_;
}
