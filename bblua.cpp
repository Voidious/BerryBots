/*
  Copyright (C) 2012-2015 - Voidious

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

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <time.h>
#include "basedir.h"
#include "bbutil.h"
#include "filemanager.h"
#include "stage.h"
#include "sensorhandler.h"
#include "bbengine.h"
#include "replaybuilder.h"
#include "printhandler.h"
#include "gamerunner.h"
#include "bbrunner.h"
#include "bblua.h"

// TODO: Consider moving some stuff between stage and engine.
// TODO: Consider adding stage pointer to StageBuilder and Admin, for speed.

// Note: The luaL_error's in this file are from within C functions being called
//       by Lua (C -> Lua -> C). They are not fatal to the host app and are
//       handled appropriately (CLI vs GUI) by the original C caller.

void luaSrand(lua_State *L) {
  char *luaSrand = new char[100];
  sprintf(luaSrand, "math.randomseed(%d)", rand());
  luaL_dostring(L, luaSrand);
  delete luaSrand;
}

void killHook(lua_State *L, lua_Debug *ar) {
  luaL_error(L, "Interrupted for using too much CPU time.");
}

void abortHook(lua_State *L, lua_Debug *ar) {
  luaL_error(L, "Game Runner aborted.");
}

void initStageState(lua_State **stageState, const char *stageCwd) {
  *stageState = luaL_newstate();
  lua_setcwd(*stageState, stageCwd);
  luaL_openlibs(*stageState);
  luaSrand(*stageState);
  registerStageBuilder(*stageState);
  registerShip(*stageState);
  registerWorld(*stageState);
  registerAdmin(*stageState);
  registerStageSensors(*stageState);
  registerStageGfx(*stageState);
  registerStageGlobals(*stageState);
}

void initShipState(lua_State **shipState, const char *shipCwd) {
  *shipState = luaL_newstate();
  lua_setcwd(*shipState, shipCwd);
  luaL_openlibs(*shipState);
  luaSrand(*shipState);
  registerShip(*shipState);
  registerSensors(*shipState);
  registerWorld(*shipState);
  registerShipGfx(*shipState);
  registerShipGlobals(*shipState);
}

void initRunnerState(lua_State **runnerState, const char *runnerCwd) {
  *runnerState = luaL_newstate();
  lua_setcwd(*runnerState, runnerCwd);
  luaL_openlibs(*runnerState);
  luaSrand(*runnerState);
  registerRunnerForm(*runnerState);
  registerGameRunner(*runnerState);
  registerRunnerFiles(*runnerState);
  registerRunnerGlobals(*runnerState);
}

void setField(lua_State *L, const char *key, double value) {
  lua_pushstring(L, key);
  lua_pushnumber(L, value);
  lua_settable(L, -3);
}

void setField(lua_State *L, const char *key, int value) {
  lua_pushstring(L, key);
  lua_pushinteger(L, value);
  lua_settable(L, -3);
}

void setField(lua_State *L, const char *key, bool value) {
  lua_pushstring(L, key);
  lua_pushboolean(L, value);
  lua_settable(L, -3);
}

void setField(lua_State *L, const char *key, const char *value) {
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_settable(L, -3);
}

void setFieldNil(lua_State *L, const char *key) {
  lua_pushstring(L, key);
  lua_pushnil(L);
  lua_settable(L, -3);
}

int registerClass(
    lua_State *L, const char *className, const luaL_Reg *methods) {
  luaL_openlib(L, className, methods, 0);
  luaL_newmetatable(L, className);
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);
  lua_pop(L, 2);
  return 1;
}

// Adapted from base Lua's luaB_print.
const char *getPrintStr(lua_State *L) {
  int top = lua_gettop(L);
  std::string s = "";
  lua_getglobal(L, "tostring");
  for (int x = 0; x < top; x++) {
    if (x > 0) {
      s.append("\t");
    }
    lua_pushvalue(L, -1);
    lua_pushvalue(L, x + 1);
    lua_call(L, 1, 1);
    s.append(lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  lua_pushstring(L, s.c_str());
  return lua_tostring(L, top + 1);
}

Ship* checkShip(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  Ship *ship = (Ship *) luaL_checkudata(L, index, SHIP);
  if (ship == NULL) luaL_error(L, "error in checkShip");
  return ship;
}

Ship* pushShip(lua_State *L) {
  Ship *ship = (Ship *) lua_newuserdata(L, sizeof(Ship));
  ship->properties = new ShipProperties;
  luaL_getmetatable(L, SHIP);
  lua_setmetatable(L, -2);
  int shipRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, shipRef);

  return ship;
}

int Ship_fireThruster(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (ship->alive && ship->thrusterEnabled) {
    ship->thrusterAngle = luaL_checknumber(L, 2);
    ship->thrusterForce = limit(0, luaL_checknumber(L, 3), MAX_THRUSTER_FORCE);
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int Ship_fireLaser(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (ship->alive && ship->laserEnabled
      && ship->properties->engine->getStage()->fireLaser(
          ship, luaL_checknumber(L, 2),
          ship->properties->engine->getGameTime())) {
    ship->laserGunHeat = LASER_HEAT;
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int Ship_fireTorpedo(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (ship->alive && ship->torpedoEnabled
      && ship->properties->engine->getStage()->fireTorpedo(
          ship, luaL_checknumber(L, 2),
          std::max(0.0, (double) luaL_checknumber(L, 3)),
          ship->properties->engine->getGameTime())) {
    ship->torpedoGunHeat = TORPEDO_HEAT;
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int Ship_x(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushnumber(L, ship->x);
  return 1;
}

int Ship_y(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushnumber(L, ship->y);
  return 1;
}

int Ship_heading(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushnumber(L, ship->heading);
  return 1;
}

int Ship_speed(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushnumber(L, ship->speed);
  return 1;
}

int Ship_energy(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushnumber(L, ship->energy);
  return 1;
}

int Ship_laserGunHeat(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushinteger(L, ship->laserGunHeat);
  return 1;
}

int Ship_torpedoGunHeat(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushinteger(L, ship->torpedoGunHeat);
  return 1;
}

int Ship_hitWall(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushboolean(L, ship->hitWall);
  return 1;
}

int Ship_hitShip(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushboolean(L, ship->hitShip);
  return 1;
}

int Ship_alive(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushboolean(L, ship->alive);
  return 1;
}

int Ship_isStageShip(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushboolean(L, ship->properties->stageShip);
  return 1;
}

void setTeamName(Team *team, const char *name) {
  strncpy(team->name, name, MAX_NAME_LENGTH);
  team->name[MAX_NAME_LENGTH] = '\0';
}

int Ship_setName(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  const char *shipName = luaL_checkstring(L, 2);
  BerryBotsEngine *engine = ship->properties->engine;
  Team *team = engine->getTeam(ship->teamIndex);
  if (!engine->isShipInitComplete() || team->gfxEnabled) {
    strncpy(ship->properties->name, shipName, MAX_NAME_LENGTH);
    ship->properties->name[MAX_NAME_LENGTH] = '\0';
    if (team->numShips == 1) {
      setTeamName(team, shipName);
    }

    std::stringstream ss;
    if (team->numShips == 1) {
      ss << "== Set";
    } else {
      ss << "== Ship " << (ship->index - team->firstShipIndex + 1) << " set";
    }
    ss << " name: " << shipName;
    engine->shipPrint(L, ss.str().c_str());
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setTeamName(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  const char *teamName = luaL_checkstring(L, 2);
  BerryBotsEngine *engine = ship->properties->engine;
  Team *team = engine->getTeam(ship->teamIndex);
  if (!engine->isShipInitComplete() || team->gfxEnabled) {
    setTeamName(team, teamName);

    std::stringstream ss;
    ss << "== Set team name: " << teamName;
    engine->shipPrint(L, ss.str().c_str());
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setShipColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  BerryBotsEngine *engine = ship->properties->engine;
  Team *team = engine->getTeam(ship->teamIndex);
  if (!engine->isShipInitComplete() || team->gfxEnabled) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->shipR = r;
    ship->properties->shipG = g;
    ship->properties->shipB = b;
    ship->newColors = true;

    std::stringstream ss;
    if (team->numShips == 1) {
      ss << "== Set";
    } else {
      ss << "== Ship " << (ship->index - team->firstShipIndex + 1) << " set";
    }
    ss << " ship color: (" << r << ", " << g << ", " << b << ")";
    engine->shipPrint(L, ss.str().c_str());
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setLaserColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  BerryBotsEngine *engine = ship->properties->engine;
  Team *team = engine->getTeam(ship->teamIndex);
  if (!engine->isShipInitComplete() || team->gfxEnabled) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->laserR = r;
    ship->properties->laserG = g;
    ship->properties->laserB = b;
    ship->newColors = true;

    std::stringstream ss;
    if (team->numShips == 1) {
      ss << "== Set";
    } else {
      ss << "== Ship " << (ship->index - team->firstShipIndex + 1) << " set";
    }
    ss << " laser color: (" << r << ", " << g << ", " << b << ")";
    engine->shipPrint(L, ss.str().c_str());
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setThrusterColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  BerryBotsEngine *engine = ship->properties->engine;
  Team *team = engine->getTeam(ship->teamIndex);
  if (!engine->isShipInitComplete() || team->gfxEnabled) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->thrusterR = r;
    ship->properties->thrusterG = g;
    ship->properties->thrusterB = b;
    ship->newColors = true;

    std::stringstream ss;
    if (team->numShips == 1) {
      ss << "== Set";
    } else {
      ss << "== Ship " << (ship->index - team->firstShipIndex + 1) << " set";
    }
    ss << " thruster color: (" << r << ", " << g << ", " << b << ")";
    engine->shipPrint(L, ss.str().c_str());
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_name(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushstring(L, ship->properties->name);
  return 1;
}

int Ship_teamName(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  lua_pushstring(L, ship->properties->engine->getTeam(ship->teamIndex)->name);
  return 1;
}

const luaL_Reg Ship_methods[] = {
  {"fireThruster",      Ship_fireThruster},
  {"fireLaser",         Ship_fireLaser},
  {"fireTorpedo",       Ship_fireTorpedo},
  {"x",                 Ship_x},
  {"y",                 Ship_y},
  {"heading",           Ship_heading},
  {"speed",             Ship_speed},
  {"energy",            Ship_energy},
  {"laserGunHeat",      Ship_laserGunHeat},
  {"torpedoGunHeat",    Ship_torpedoGunHeat},
  {"hitWall",           Ship_hitWall},
  {"hitShip",           Ship_hitShip},
  {"alive",             Ship_alive},
  {"isStageShip",       Ship_isStageShip},
  {"setName",           Ship_setName},
  {"setTeamName",       Ship_setTeamName},
  {"setShipColor",      Ship_setShipColor},
  {"setLaserColor",     Ship_setLaserColor},
  {"setThrusterColor",  Ship_setThrusterColor},
  {"name",              Ship_name},
  {"teamName",          Ship_teamName},
  {0, 0}
};

int registerShip(lua_State *L) {
  return registerClass(L, SHIP, Ship_methods);
}

void pushVisibleEnemyShips(
    lua_State *L, bool *teamVision, int teamIndex, Ship **ships, int numShips) {
  lua_newtable(L);
  int visibleIndex = 1;
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->teamIndex != teamIndex && teamVision[x]) {
      lua_newtable(L);
      setField(L, "x", ship->x);
      setField(L, "y", ship->y);
      setField(L, "heading", ship->heading);
      setField(L, "speed", ship->speed);
      setField(L, "energy", ship->energy);
      setField(L, "isStageShip", ship->properties->stageShip);
      setField(L, "name", ship->properties->name);
      setField(L, "teamName",
               ship->properties->engine->getTeam(ship->teamIndex)->name);
      lua_rawseti(L, -2, visibleIndex++);
    }
  }
}

Sensors* checkSensors(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  Sensors *sensors = (Sensors *) luaL_checkudata(L, index, SENSORS);
  if (sensors == NULL) luaL_error(L, "error in checkSensors");
  return sensors;
}

Sensors* pushSensors(
    Team *team, SensorHandler *sensorHandler, ShipProperties **properties) {
  lua_State *L = team->state;
  Sensors *sensors = (Sensors *) lua_newuserdata(L, sizeof(Sensors));
  luaL_getmetatable(L, SENSORS);
  lua_setmetatable(L, -2);
  int teamIndex = team->index;

  lua_newtable(L);
  int numHitByShips = sensorHandler->numHitByShips(teamIndex);
  HitByShip** hitByShipEvents = sensorHandler->getHitByShips(teamIndex);
  for (int x = 0; x < numHitByShips; x++) {
    HitByShip* hitByShip = hitByShipEvents[x];
    lua_newtable(L);
    setField(L, "time", hitByShip->time);
    setField(L, "targetName", properties[hitByShip->targetShipIndex]->name);
    setField(L, "targetX", hitByShip->targetX);
    setField(L, "targetY", hitByShip->targetY);
    setField(L, "hittingName", properties[hitByShip->hittingShipIndex]->name);
    setField(L, "hittingX", hitByShip->hittingX);
    setField(L, "hittingY", hitByShip->hittingY);
    setField(L, "inAngle", hitByShip->inAngle);
    setField(L, "inForce", hitByShip->inForce);
    setField(L, "outAngle", hitByShip->outAngle);
    setField(L, "outForce", hitByShip->outForce);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->hitByShipRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearHitByShips(teamIndex);

  lua_newtable(L);
  int numHitByLasers = sensorHandler->numHitByLasers(teamIndex);
  HitByLaser** hitByLaserEvents = sensorHandler->getHitByLasers(teamIndex);
  for (int x = 0; x < numHitByLasers; x++) {
    HitByLaser* hitByLaser = hitByLaserEvents[x];
    lua_newtable(L);
    setField(L, "time", hitByLaser->time);
    setField(L, "targetName", properties[hitByLaser->targetShipIndex]->name);
    setField(L, "laserX", hitByLaser->laserX);
    setField(L, "laserY", hitByLaser->laserY);
    setField(L, "laserHeading", hitByLaser->laserHeading);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->hitByLaserRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearHitByLasers(teamIndex);

  lua_newtable(L);
  int numHitByTorpedos = sensorHandler->numHitByTorpedos(teamIndex);
  HitByTorpedo** hitByTorpedoEvents = sensorHandler->getHitByTorpedos(teamIndex);
  for (int x = 0; x < numHitByTorpedos; x++) {
    HitByTorpedo* hitByTorpedo = hitByTorpedoEvents[x];
    lua_newtable(L);
    setField(L, "time", hitByTorpedo->time);
    setField(L, "targetName", properties[hitByTorpedo->targetShipIndex]->name);
    setField(L, "hitAngle", hitByTorpedo->hitAngle);
    setField(L, "hitForce", hitByTorpedo->hitForce);
    setField(L, "hitDamage", hitByTorpedo->hitDamage);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->hitByTorpedoRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearHitByTorpedos(teamIndex);

  lua_newtable(L);
  int numHitWalls = sensorHandler->numHitWalls(teamIndex);
  ShipHitWall** hitWallEvents = sensorHandler->getHitWalls(teamIndex);
  for (int x = 0; x < numHitWalls; x++) {
    ShipHitWall* hitWall = hitWallEvents[x];
    lua_newtable(L);
    setField(L, "time", hitWall->time);
    setField(L, "shipName", properties[hitWall->shipIndex]->name);
    setField(L, "shipX", hitWall->shipX);
    setField(L, "shipY", hitWall->shipY);
    setField(L, "bounceAngle", hitWall->bounceAngle);
    setField(L, "bounceForce", hitWall->bounceForce);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->hitWallRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearHitWalls(teamIndex);

  lua_newtable(L);
  int numShipDestroyeds = sensorHandler->numShipDestroyeds(teamIndex);
  ShipDestroyed** shipDestroyedEvents =
      sensorHandler->getShipDestroyeds(teamIndex);
  for (int x = 0; x < numShipDestroyeds; x++) {
    ShipDestroyed* shipDestroyed = shipDestroyedEvents[x];
    lua_newtable(L);
    setField(L, "time", shipDestroyed->time);
    setField(L, "shipName", properties[shipDestroyed->shipIndex]->name);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->shipDestroyedRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearShipDestroyeds(teamIndex);

  lua_newtable(L);
  int numShipFiredLasers = sensorHandler->numShipFiredLasers(teamIndex);
  ShipFiredLaser** shipFiredLaserEvents =
      sensorHandler->getShipFiredLasers(teamIndex);
  for (int x = 0; x < numShipFiredLasers; x++) {
    ShipFiredLaser* shipFiredLaser = shipFiredLaserEvents[x];
    lua_newtable(L);
    setField(L, "time", shipFiredLaser->time);
    setField(L, "shipName", properties[shipFiredLaser->shipIndex]->name);
    setField(L, "shipX", shipFiredLaser->shipX);
    setField(L, "shipY", shipFiredLaser->shipY);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->shipFiredLaserRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearShipFiredLasers(teamIndex);

  lua_newtable(L);
  int numShipFiredTorpedos = sensorHandler->numShipFiredTorpedos(teamIndex);
  ShipFiredTorpedo** shipFiredTorpedoEvents =
      sensorHandler->getShipFiredTorpedos(teamIndex);
  for (int x = 0; x < numShipFiredTorpedos; x++) {
    ShipFiredTorpedo* shipFiredTorpedo = shipFiredTorpedoEvents[x];
    lua_newtable(L);
    setField(L, "time", shipFiredTorpedo->time);
    setField(L, "shipName", properties[shipFiredTorpedo->shipIndex]->name);
    setField(L, "shipX", shipFiredTorpedo->shipX);
    setField(L, "shipY", shipFiredTorpedo->shipY);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->shipFiredTorpedoRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearShipFiredTorpedos(teamIndex);

  lua_newtable(L);
  int numLaserHitShips = sensorHandler->numLaserHitShips(teamIndex);
  LaserHitShip** laserHitShipEvents =
      sensorHandler->getLaserHitShips(teamIndex);
  for (int x = 0; x < numLaserHitShips; x++) {
    LaserHitShip* laserHitShip = laserHitShipEvents[x];
    lua_newtable(L);
    setField(L, "time", laserHitShip->time);
    setField(L, "targetName", properties[laserHitShip->targetShipIndex]->name);
    setField(L, "targetX", laserHitShip->shipX);
    setField(L, "targetY", laserHitShip->shipY);
    lua_rawseti(L, -2, x + 1);
  }
  sensors->laserHitShipRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearLaserHitShips(teamIndex);

  if (team->stageEventRef == 0) {
    lua_newtable(L);
    sensors->stageEventRef = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    sensors->stageEventRef = team->stageEventRef;
  }
  team->stageEventRef = 0;

  return sensors;
}

void cleanupSensorsTables(lua_State *L, Sensors *sensors) {
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->hitByShipRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->hitByLaserRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->hitByTorpedoRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->hitWallRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->shipDestroyedRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->shipFiredLaserRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->shipFiredTorpedoRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->laserHitShipRef);
  luaL_unref(L, LUA_REGISTRYINDEX, sensors->stageEventRef);
  sensors->stageEventRef = 0;
}

int Sensors_hitByShipEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->hitByShipRef);
  return 1;
}

int Sensors_hitByLaserEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->hitByLaserRef);
  return 1;
}

int Sensors_hitByTorpedoEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->hitByTorpedoRef);
  return 1;
}

int Sensors_hitWallEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->hitWallRef);
  return 1;
}

int Sensors_shipDestroyedEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->shipDestroyedRef);
  return 1;
}

int Sensors_shipFiredLaserEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->shipFiredLaserRef);
  return 1;
}

int Sensors_shipFiredTorpedoEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->shipFiredTorpedoRef);
  return 1;
}

int Sensors_laserHitShipEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->laserHitShipRef);
  return 1;
}

int Sensors_stageEvents(lua_State *L) {
  Sensors *sensors = checkSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, sensors->stageEventRef);
  return 1;
}

const luaL_Reg Sensors_methods[] = {
  {"hitByShipEvents",         Sensors_hitByShipEvents},
  {"hitByLaserEvents",        Sensors_hitByLaserEvents},
  {"hitByTorpedoEvents",      Sensors_hitByTorpedoEvents},
  {"hitWallEvents",           Sensors_hitWallEvents},
  {"shipDestroyedEvents",     Sensors_shipDestroyedEvents},
  {"shipFiredLaserEvents",    Sensors_shipFiredLaserEvents},
  {"shipFiredTorpedoEvents",  Sensors_shipFiredTorpedoEvents},
  {"laserHitShipEvents",      Sensors_laserHitShipEvents},
  {"stageEvents",             Sensors_stageEvents},
  {0, 0}
};

int registerSensors(lua_State *L) {
  return registerClass(L, SENSORS, Sensors_methods);
}

StageSensors* checkStageSensors(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  StageSensors *stageSensors =
      (StageSensors *) luaL_checkudata(L, index, STAGE_SENSORS);
  if (stageSensors == NULL) luaL_error(L, "error in checkStageSensors");
  return stageSensors;
}

StageSensors* pushStageSensors(lua_State *L, SensorHandler *sensorHandler,
                               Ship **ships, ShipProperties **properties) {
  StageSensors *stageSensors =
      (StageSensors *) lua_newuserdata(L, sizeof(StageSensors));
  luaL_getmetatable(L, STAGE_SENSORS);
  lua_setmetatable(L, -2);

  lua_newtable(L);
  int numShipHitShips = sensorHandler->numStageShipHitShips();
  ShipHitShip** shipHitShipEvents = sensorHandler->getStageShipHitShips();
  for (int x = 0; x < numShipHitShips; x++) {
    ShipHitShip* shipHitShip = shipHitShipEvents[x];
    lua_newtable(L);
    setField(L, "time", shipHitShip->time);
    setField(L, "targetName",
             properties[shipHitShip->targetShipIndex]->name);
    setField(L, "targetX", shipHitShip->targetX);
    setField(L, "targetY", shipHitShip->targetY);
    setField(L, "hittingName",
             properties[shipHitShip->hittingShipIndex]->name);
    setField(L, "hittingX", shipHitShip->hittingX);
    setField(L, "hittingY", shipHitShip->hittingY);
    setField(L, "inAngle", shipHitShip->inAngle);
    setField(L, "inForce", shipHitShip->inForce);
    setField(L, "outAngle", shipHitShip->outAngle);
    setField(L, "outForce", shipHitShip->outForce);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->shipHitShipRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageShipHitShips();

  lua_newtable(L);
  int numLaserHitShips = sensorHandler->numStageLaserHitShips();
  StageLaserHitShip** laserHitShipEvents =
      sensorHandler->getStageLaserHitShips();
  for (int x = 0; x < numLaserHitShips; x++) {
    StageLaserHitShip* laserHitShip = laserHitShipEvents[x];
    lua_newtable(L);
    setField(L, "time", laserHitShip->time);
    setField(L, "srcName", properties[laserHitShip->srcShipIndex]->name);
    setField(L, "targetName", properties[laserHitShip->targetShipIndex]->name);
    setField(L, "laserX", laserHitShip->laserX);
    setField(L, "laserY", laserHitShip->laserY);
    setField(L, "laserHeading", laserHitShip->laserHeading);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->laserHitShipRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageLaserHitShips();

  lua_newtable(L);
  int numTorpedoHitShips = sensorHandler->numStageTorpedoHitShips();
  StageTorpedoHitShip** torpedoHitShipEvents =
      sensorHandler->getStageTorpedoHitShips();
  for (int x = 0; x < numTorpedoHitShips; x++) {
    StageTorpedoHitShip* torpedoHitShip = torpedoHitShipEvents[x];
    lua_newtable(L);
    setField(L, "time", torpedoHitShip->time);
    setField(L, "srcName", properties[torpedoHitShip->srcShipIndex]->name);
    setField(L, "targetName",
        properties[torpedoHitShip->targetShipIndex]->name);
    setField(L, "hitAngle", torpedoHitShip->hitAngle);
    setField(L, "hitForce", torpedoHitShip->hitForce);
    setField(L, "hitDamage", torpedoHitShip->hitDamage);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->torpedoHitShipRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageTorpedoHitShips();

  lua_newtable(L);
  int numShipHitWalls = sensorHandler->numStageShipHitWalls();
  ShipHitWall** shipHitWallEvents = sensorHandler->getStageShipHitWalls();
  for (int x = 0; x < numShipHitWalls; x++) {
    ShipHitWall* shipHitWall = shipHitWallEvents[x];
    lua_newtable(L);
    setField(L, "time", shipHitWall->time);
    setField(L, "shipName", properties[shipHitWall->shipIndex]->name);
    setField(L, "shipX", shipHitWall->shipX);
    setField(L, "shipY", shipHitWall->shipY);
    setField(L, "bounceAngle", shipHitWall->bounceAngle);
    setField(L, "bounceForce", shipHitWall->bounceForce);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->shipHitWallRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageShipHitWalls();

  lua_newtable(L);
  int numShipDestroyeds = sensorHandler->numStageShipDestroyeds();
  ShipDestroyed** shipDestroyedEvents = sensorHandler->getStageShipDestroyeds();
  for (int x = 0; x < numShipDestroyeds; x++) {
    ShipDestroyed* shipDestroyed = shipDestroyedEvents[x];
    BerryBotsEngine *engine = properties[shipDestroyed->shipIndex]->engine;
    if (shipDestroyed->time == engine->getGameTime()) {
      // TODO: doesn't seem like we should need this check - do we?
      lua_newtable(L);
      setField(L, "time", shipDestroyed->time);
      setField(L, "shipName", properties[shipDestroyed->shipIndex]->name);
      lua_rawseti(L, -2, x + 1);
    }
  }
  stageSensors->shipDestroyedRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageShipDestroyeds();

  lua_newtable(L);
  int numShipFiredLasers = sensorHandler->numStageShipFiredLasers();
  StageShipFiredLaser** shipFiredLaserEvents =
      sensorHandler->getStageShipFiredLasers();
  for (int x = 0; x < numShipFiredLasers; x++) {
    StageShipFiredLaser* shipFiredLaser = shipFiredLaserEvents[x];
    lua_newtable(L);
    setField(L, "time", shipFiredLaser->time);
    setField(L, "shipName", properties[shipFiredLaser->shipIndex]->name);
    setField(L, "shipX", shipFiredLaser->shipX);
    setField(L, "shipY", shipFiredLaser->shipY);
    setField(L, "laserHeading", shipFiredLaser->laserHeading);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->shipFiredLaserRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageShipFiredLasers();

  lua_newtable(L);
  int numShipFiredTorpedos = sensorHandler->numStageShipFiredTorpedos();
  StageShipFiredTorpedo** shipFiredTorpedoEvents =
      sensorHandler->getStageShipFiredTorpedos();
  for (int x = 0; x < numShipFiredTorpedos; x++) {
    StageShipFiredTorpedo* shipFiredTorpedo = shipFiredTorpedoEvents[x];
    lua_newtable(L);
    setField(L, "time", shipFiredTorpedo->time);
    setField(L, "shipName", properties[shipFiredTorpedo->shipIndex]->name);
    setField(L, "shipX", shipFiredTorpedo->shipX);
    setField(L, "shipY", shipFiredTorpedo->shipY);
    setField(L, "torpedoHeading", shipFiredTorpedo->torpedoHeading);
    setField(L, "torpedoDistance", shipFiredTorpedo->torpedoDistance);
    lua_rawseti(L, -2, x + 1);
  }
  stageSensors->shipFiredTorpedoRef = luaL_ref(L, LUA_REGISTRYINDEX);
  sensorHandler->clearStageShipFiredTorpedos();

  return stageSensors;
}

void cleanupStageSensorsTables(
    lua_State *L, StageSensors *stageSensors) {
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->shipHitShipRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->laserHitShipRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->torpedoHitShipRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->shipHitWallRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->shipDestroyedRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->shipFiredLaserRef);
  luaL_unref(L, LUA_REGISTRYINDEX, stageSensors->shipFiredTorpedoRef);
}

int StageSensors_shipHitShipEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->shipHitShipRef);
  return 1;
}

int StageSensors_laserHitShipEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->laserHitShipRef);
  return 1;
}

int StageSensors_torpedoHitShipEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->torpedoHitShipRef);
  return 1;
}

int StageSensors_shipHitWallEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->shipHitWallRef);
  return 1;
}

int StageSensors_shipDestroyedEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->shipDestroyedRef);
  return 1;
}

int StageSensors_shipFiredLaserEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->shipFiredLaserRef);
  return 1;
}

int StageSensors_shipFiredTorpedoEvents(lua_State *L) {
  StageSensors *stageSensors = checkStageSensors(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageSensors->shipFiredTorpedoRef);
  return 1;
}

const luaL_Reg StageSensors_methods[] = {
  {"shipHitShipEvents",     StageSensors_shipHitShipEvents},
  {"laserHitShipEvents",    StageSensors_laserHitShipEvents},
  {"torpedoHitShipEvents",  StageSensors_torpedoHitShipEvents},
  {"shipHitWallEvents",     StageSensors_shipHitWallEvents},
  {"shipDestroyedEvents",   StageSensors_shipDestroyedEvents},
  {"shipFiredLaserEvents",  StageSensors_shipFiredLaserEvents},
  {"shipFiredTorpedoEvents",  StageSensors_shipFiredTorpedoEvents},
  {0, 0}
};

int registerStageSensors(lua_State *L) {
  return registerClass(L, STAGE_SENSORS, StageSensors_methods);
}

StageBuilder* checkStageBuilder(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  StageBuilder *stageBuilder =
      (StageBuilder *) luaL_checkudata(L, index, STAGE_BUILDER);
  if (stageBuilder == NULL) luaL_error(L, "error in checkStageBuilder");
  return stageBuilder;
}

StageBuilder* pushStageBuilder(lua_State *L) {
  StageBuilder *stageBuilder =
      (StageBuilder *) lua_newuserdata(L, sizeof(StageBuilder));
  luaL_getmetatable(L, STAGE_BUILDER);
  lua_setmetatable(L, -2);
  int stageBuilderRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageBuilderRef);
  return stageBuilder;
}

int StageBuilder_setSize(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int width = std::max(SHIP_SIZE, luaL_checkint(L, 2));
  int height = std::max(SHIP_SIZE, luaL_checkint(L, 3));

  BerryBotsEngine *engine = stageBuilder->engine;
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set stage size outside of 'configure' function.");
  } else {
    engine->getStage()->setSize(width, height);

    std::stringstream ss;
    ss << "== Set stage size: " << width << " x " << height;
    engine->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_setBattleMode(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  bool battleMode = lua_toboolean(L, 2);

  BerryBotsEngine *engine = stageBuilder->engine;
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set battle mode outside of 'configure' function.");
  } else {
    engine->setBattleMode(battleMode);
    std::stringstream ss;
    ss << "== Set battle mode: " << (battleMode ? "true" : "false");
    engine->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_addWall(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int left = luaL_checkint(L, 2);
  int bottom = luaL_checkint(L, 3);
  int width = std::max(0, luaL_checkint(L, 4));
  int height = std::max(0, luaL_checkint(L, 5));
  BerryBotsEngine *engine = stageBuilder->engine;
  Stage *stage = engine->getStage();
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add walls outside of 'configure' function.");
  } else if (!stage->addWall(left, bottom, width, height, true)) {
    luaL_error(L, "Failed to add wall - is %i too many?",
               stage->getWallCount());
  }

  std::stringstream ss;
  ss << "== Added wall: bottom left (" << left << ", " << bottom << "), "
     << "dimensions " << width << " x " << height;
  engine->stagePrint(ss.str().c_str());
  return 1;
}

int StageBuilder_addStart(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  BerryBotsEngine *engine = stageBuilder->engine;
  Stage *stage = engine->getStage();
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add starts outside of 'configure' function.");
  } else if (!stage->addStart(x, y)) {
    luaL_error(L, "Failed to add start - is %i too many?",
               stage->getStartCount());
  }

  std::stringstream ss;
  ss << "== Added start position: (" << x << ", " << y << ")";
  engine->stagePrint(ss.str().c_str());
  return 1;
}

int StageBuilder_addZone(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int left = luaL_checkint(L, 2);
  int bottom = luaL_checkint(L, 3);
  int width = std::max(0, luaL_checkint(L, 4));
  int height = std::max(0, luaL_checkint(L, 5));
  const char *zoneTag = luaL_optstring(L, 6, "");
  BerryBotsEngine *engine = stageBuilder->engine;
  Stage *stage = engine->getStage();
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add zones outside of 'configure' function.");
  } else if (!stage->addZone(left, bottom, width, height, zoneTag)) {
    luaL_error(L, "Failed to add zone - is %i too many?",
               stage->getZoneCount());
  }

  std::stringstream ss;
  ss << "== Added zone: bottom left (" << left << ", " << bottom << "), "
     << "dimensions: " << width << " x " << height;
  engine->stagePrint(ss.str().c_str());
  return 1;
}

int StageBuilder_addShip(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  const char *stageShipFilename = luaL_checkstring(L, 2);
  BerryBotsEngine *engine = stageBuilder->engine;
  Stage *stage = engine->getStage();
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add stage ships outside of 'configure' function.");
  } else if (!stage->addStageShip(stageShipFilename)) {
    luaL_error(L, "Failed to add stage ship - is %i too many?",
               stage->getStageShipCount());
  }

  std::stringstream ss;
  ss << "== Added stage ship: " << stageShipFilename;
  engine->stagePrint(ss.str().c_str());
  return 1;
}

int StageBuilder_setTeamSize(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int teamSize = luaL_checkint(L, 2);
  BerryBotsEngine *engine = stageBuilder->engine;
  if (engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set team size outside of 'configure' function.");
  } else {
    engine->setTeamSize(teamSize);
    std::stringstream ss;
    ss << "== Set team size: " << teamSize;
    engine->stagePrint(ss.str().c_str());
  }
  return 1;
}

const luaL_Reg StageBuilder_methods[] = {
  {"setSize",        StageBuilder_setSize},
  {"setBattleMode",  StageBuilder_setBattleMode},
  {"addWall",        StageBuilder_addWall},
  {"addStart",       StageBuilder_addStart},
  {"addZone",        StageBuilder_addZone},
  {"addShip",        StageBuilder_addShip},
  {"setTeamSize",    StageBuilder_setTeamSize},
  {0, 0}
};

int registerStageBuilder(lua_State *L) {
  return registerClass(L, STAGE_BUILDER, StageBuilder_methods);
}

void pushWalls(lua_State *L, Wall** walls, int wallCount) {
  lua_newtable(L);
  for (int x = 0; x < wallCount; x++) {
    Wall *wall = walls[x];
    lua_newtable(L);
    setField(L, "left", wall->getLeft());
    setField(L, "bottom", wall->getBottom());
    setField(L, "width", wall->getWidth());
    setField(L, "height", wall->getHeight());
    lua_rawseti(L, -2, x + 1);
  }
}

void pushZones(lua_State *L, Zone** zones, int zoneCount) {
  lua_newtable(L);
  for (int x = 0; x < zoneCount; x++) {
    Zone *zone = zones[x];
    lua_newtable(L);
    setField(L, "left", zone->getLeft());
    setField(L, "bottom", zone->getBottom());
    setField(L, "width", zone->getWidth());
    setField(L, "height", zone->getHeight());
    if (zone->hasTag()) {
      setField(L, "tag", zone->getTag());
    } else {
      setField(L, "tag", "");
    }
    lua_rawseti(L, -2, x + 1);
  }
}

World* checkWorld(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  World *world = (World*) luaL_checkudata(L, index, WORLD);
  if (world == NULL) luaL_error(L, "error in checkWorld");
  return world;
}

World* pushWorld(lua_State *L, Stage *stage, int numShips, int teamSize) {
  World *world = (World *) lua_newuserdata(L, sizeof(World));
  luaL_getmetatable(L, WORLD);
  lua_setmetatable(L, -2);
  int worldRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, worldRef);
  world->width = stage->getWidth();
  world->height = stage->getHeight();
  world->numShips = numShips;
  world->teamSize = teamSize;
  world->time = 0;

  pushWalls(L, stage->getWalls(), stage->getWallCount());
  world->wallsRef = luaL_ref(L, LUA_REGISTRYINDEX);

  pushZones(L, stage->getZones(), stage->getZoneCount());
  world->zonesRef = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_newtable(L);
  setField(L, "SHIP_RADIUS", SHIP_RADIUS);
  setField(L, "LASER_SPEED", LASER_SPEED);
  setField(L, "LASER_HEAT", LASER_HEAT);
  setField(L, "LASER_DAMAGE", LASER_DAMAGE);
  setField(L, "TORPEDO_SPEED", TORPEDO_SPEED);
  setField(L, "TORPEDO_HEAT", TORPEDO_HEAT);
  setField(L, "TORPEDO_BLAST_RADIUS", TORPEDO_BLAST_RADIUS);
  setField(L, "TORPEDO_BLAST_FORCE", TORPEDO_BLAST_FORCE);
  setField(L, "TORPEDO_BLAST_DAMAGE", TORPEDO_BLAST_DAMAGE);
  setField(L, "DEFAULT_ENERGY", DEFAULT_ENERGY);
  setField(L, "MAX_THRUSTER_FORCE", MAX_THRUSTER_FORCE);
  setField(L, "WALL_BOUNCE", WALL_BOUNCE);
  world->constantsRef = luaL_ref(L, LUA_REGISTRYINDEX);

  return world;
}

void pushCopyOfShips(
    lua_State *L, Ship** ships, Ship **shipsCopy, int numShips) {
  lua_newtable(L);
  for (int x = 0; x < numShips; x++) {
    shipsCopy[x] = pushShip(L);
    *(shipsCopy[x]) = *(ships[x]);
    lua_rawseti(L, -2, x + 1);
  }
}

int World_constants(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, world->constantsRef);
  return 1;
}

int World_walls(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, world->wallsRef);
  return 1;
}

int World_zones(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, world->zonesRef);
  return 1;
}

int World_width(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_pushinteger(L, world->width);
  return 1;
}

int World_height(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_pushinteger(L, world->height);
  return 1;
}

int World_time(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_pushinteger(L, world->time);
  return 1;
}

int World_numShips(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_pushinteger(L, world->numShips);
  return 1;
}

int World_teamSize(lua_State *L) {
  World *world = checkWorld(L, 1);
  lua_pushinteger(L, world->teamSize);
  return 1;
}

int World_inAnyZone(lua_State *L) {
  World *world = checkWorld(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushboolean(L, world->engine->getStage()->inAnyZone(ship));
  return 1;
}

int World_inZone(lua_State *L) {
  World *world = checkWorld(L, 1);
  Ship *ship = checkShip(L, 2);
  const char *zoneTag = luaL_optstring(L, 3, "");
  lua_pushboolean(L, world->engine->getStage()->inZone(ship, zoneTag));
  return 1;
}

int World_touchedAnyZone(lua_State *L) {
  World *world = checkWorld(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushboolean(L, world->engine->touchedAnyZone(ship));
  return 1;
}

int World_touchedZone(lua_State *L) {
  World *world = checkWorld(L, 1);
  Ship *ship = checkShip(L, 2);
  const char *zoneTag = luaL_optstring(L, 3, "");
  lua_pushboolean(L, world->engine->touchedZone(ship, zoneTag));
  return 1;
}

const luaL_Reg World_methods[] = {
  {"constants",       World_constants},
  {"walls",           World_walls},
  {"zones",           World_zones},
  {"width",           World_width},
  {"height",          World_height},
  {"time",            World_time},
  {"numShips",        World_numShips},
  {"teamSize",        World_teamSize},
  {"inAnyZone",       World_inAnyZone},
  {"inZone",          World_inZone},
  {"touchedAnyZone",  World_touchedAnyZone},
  {"touchedZone",     World_touchedZone},
  {0, 0}
};

int registerWorld(lua_State *L) {
  return registerClass(L, WORLD, World_methods);
}

ShipGfx* checkShipGfx(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  ShipGfx *shipGfx = (ShipGfx *) luaL_checkudata(L, index, SHIP_GFX);
  if (shipGfx == NULL) luaL_error(L, "error in checkShipGfx");
  return shipGfx;
}

ShipGfx* pushShipGfx(lua_State *L) {
  ShipGfx *shipGfx = (ShipGfx *) lua_newuserdata(L, sizeof(ShipGfx));
  luaL_getmetatable(L, SHIP_GFX);
  lua_setmetatable(L, -2);
  int shipGfxRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, shipGfxRef);
  return shipGfx;
}

int getIntFromRgbaTable(lua_State *L, int index, const char *key,
                        int defaultValue) {
  lua_pushstring(L, key);
  lua_gettable(L, index);
  int value;
  if (lua_isnil(L, -1)) {
    value = defaultValue;
  } else {
    value = luaL_checkint(L, -1);
  }
  lua_pop(L, 1);
  return value;
}

RgbaColor getRgbaColor(lua_State *L, int index) {
  RgbaColor fillColor;
  fillColor.r = getIntFromRgbaTable(L, index, "r", 0);
  fillColor.g = getIntFromRgbaTable(L, index, "g", 0);
  fillColor.b = getIntFromRgbaTable(L, index, "b", 0);
  fillColor.a = getIntFromRgbaTable(L, index, "a", 255);

  return fillColor;
}

RgbaColor getTransparentColor() {
  RgbaColor color = TRANSPARENT_COLOR;
  return color;
}

RgbaColor getSolidWhiteColor() {
  RgbaColor color = SOLID_WHITE_COLOR;
  return color;
}

void addUserGfxRectangle(lua_State *L, BerryBotsEngine *engine, Team *team) {
  double left = luaL_checknumber(L, 2);
  double bottom = luaL_checknumber(L, 3);
  double width = std::max(0.0, luaL_checknumber(L, 4));
  double height = std::max(0.0, luaL_checknumber(L, 5));
  double rotation = luaL_optnumber(L, 6, 0);
  RgbaColor fillColor;
  if (lua_istable(L, 7)) {
    fillColor = getRgbaColor(L, 7);
  } else {
    fillColor = getTransparentColor();
  }
  double outlineThickness =
      std::max(0.0, luaL_optnumber(L, 8, DEFAULT_OUTLINE_THICKNESS));
  RgbaColor outlineColor;
  if (lua_istable(L, 9)) {
    outlineColor = getRgbaColor(L, 9);
  } else {
    outlineColor = getSolidWhiteColor();
  }
  int drawTicks = std::max(1, luaL_optint(L, 10, 1));

  engine->getStage()->addUserGfxRectangle(team, engine->getGameTime(), left,
      bottom, width, height, rotation, fillColor, outlineThickness,
      outlineColor, drawTicks);
}

void addUserGfxLine(lua_State *L, BerryBotsEngine *engine, Team *team) {
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double angle = luaL_checknumber(L, 4);
  double length = std::max(0.0, luaL_checknumber(L, 5));
  double thickness =
      std::max(0.0, luaL_optnumber(L, 6, DEFAULT_LINE_THICKNESS));
  RgbaColor fillColor;
  if (lua_istable(L, 7)) {
    fillColor = getRgbaColor(L, 7);
  } else {
    fillColor = getSolidWhiteColor();
  }
  double outlineThickness = std::max(0.0, luaL_optnumber(L, 8, 0));
  RgbaColor outlineColor;
  if (lua_istable(L, 9)) {
    outlineColor = getRgbaColor(L, 9);
  } else {
    outlineColor = getTransparentColor();
  }
  int drawTicks = std::max(1, luaL_optint(L, 10, 1));

  engine->getStage()->addUserGfxLine(team, engine->getGameTime(), x, y, angle,
      length, thickness, fillColor, outlineThickness, outlineColor, drawTicks);
}

void addUserGfxCircle(lua_State *L, BerryBotsEngine *engine, Team *team) {
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double radius = std::max(0.0, luaL_checknumber(L, 4));
  RgbaColor fillColor;
  if (lua_istable(L, 5)) {
    fillColor = getRgbaColor(L, 5);
  } else {
    fillColor = getTransparentColor();
  }
  double outlineThickness =
      std::max(0.0, luaL_optnumber(L, 6, DEFAULT_OUTLINE_THICKNESS));
  RgbaColor outlineColor;
  if (lua_istable(L, 7)) {
    outlineColor = getRgbaColor(L, 7);
  } else {
    outlineColor = getSolidWhiteColor();
  }
  int drawTicks = std::max(1, luaL_optint(L, 8, 1));

  engine->getStage()->addUserGfxCircle(team, engine->getGameTime(), x, y,
      radius, fillColor, outlineThickness, outlineColor, drawTicks);
}

void addUserGfxText(lua_State *L, BerryBotsEngine *engine, Team *team) {
  const char *text = luaL_checkstring(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  int fontSize = luaL_optint(L, 5, DEFAULT_SHIP_TEXT_SIZE);
  RgbaColor textColor;
  if (lua_istable(L, 6)) {
    textColor = getRgbaColor(L, 6);
  } else {
    textColor = getSolidWhiteColor();
  }
  int drawTicks = std::max(1, luaL_optint(L, 7, 1));

  engine->getStage()->addUserGfxText(team, engine->getGameTime(), text, x, y,
                                     fontSize, textColor, drawTicks);
}

int ShipGfx_drawRectangle(lua_State *L) {
  ShipGfx *shipGfx = checkShipGfx(L, 1);
  addUserGfxRectangle(L, shipGfx->engine, shipGfx->team);
  return 1;
}

int ShipGfx_drawLine(lua_State *L) {
  ShipGfx *shipGfx = checkShipGfx(L, 1);
  addUserGfxLine(L, shipGfx->engine, shipGfx->team);
  return 1;
}

int ShipGfx_drawCircle(lua_State *L) {
  ShipGfx *shipGfx = checkShipGfx(L, 1);
  addUserGfxCircle(L, shipGfx->engine, shipGfx->team);
  return 1;
}

int ShipGfx_drawText(lua_State *L) {
  ShipGfx *shipGfx = checkShipGfx(L, 1);
  addUserGfxText(L, shipGfx->engine, shipGfx->team);
  return 1;
}

int ShipGfx_enabled(lua_State *L) {
  ShipGfx *shipGfx = checkShipGfx(L, 1);
  lua_pushboolean(L, shipGfx->team->gfxEnabled);
  return 1;
}

const luaL_Reg ShipGfx_methods[] = {
  {"drawRectangle",  ShipGfx_drawRectangle},
  {"drawLine",       ShipGfx_drawLine},
  {"drawCircle",     ShipGfx_drawCircle},
  {"drawText",       ShipGfx_drawText},
  {"enabled",        ShipGfx_enabled},
  {0, 0}
};

int registerShipGfx(lua_State *L) {
  return registerClass(L, SHIP_GFX, ShipGfx_methods);
}

Admin* checkAdmin(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  Admin *admin = (Admin *) luaL_checkudata(L, index, ADMIN);
  if (admin == NULL) luaL_error(L, "error in checkAdmin");
  return admin;
}

Admin* pushAdmin(lua_State *L) {
  Admin *admin = (Admin *) lua_newuserdata(L, sizeof(Admin));
  luaL_getmetatable(L, ADMIN);
  lua_setmetatable(L, -2);
  int adminRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, adminRef);
  return admin;
}

Ship* getShip(lua_State *L, int index, BerryBotsEngine *engine) {
  if (lua_isstring(L, index)) {
    return engine->getStageProgramShip(luaL_checkstring(L, index));
  } else if (lua_isuserdata(L, index)) {
    Ship *ship = checkShip(L, index);
    return ship;
  } else {
    return 0;
  }
}

int Admin_destroyShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    admin->engine->destroyShip(ship);
  }
  return 1;
}

int Admin_reviveShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->alive = true;
    ship->energy = DEFAULT_ENERGY;
    admin->engine->getStage()->updateShipPosition(ship, ship->x, ship->y);
  }
  return 1;
}

int Admin_moveShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    admin->engine->getStage()->updateShipPosition(ship, x, y);
  }
  return 1;
}

int Admin_setShipSpeed(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->speed = luaL_checknumber(L, 3);
  }
  return 1;
}

int Admin_setShipHeading(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->heading = luaL_checknumber(L, 3);
  }
  return 1;
}

int Admin_setShipEnergy(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->energy = luaL_checknumber(L, 3);
  }
  return 1;
}

int Admin_setShipLaserEnabled(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->laserEnabled = lua_toboolean(L, 3);
  }
  return 1;
}

int Admin_setShipTorpedoEnabled(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->torpedoEnabled = lua_toboolean(L, 3);
  }
  return 1;
}

int Admin_setShipThrusterEnabled(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->thrusterEnabled = lua_toboolean(L, 3);
  }
  return 1;
}

int Admin_setShipEnergyEnabled(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->energyEnabled = lua_toboolean(L, 3);
  }
  return 1;
}

int Admin_setShipShowName(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    ship->showName = lua_toboolean(L, 3);
  }
  return 1;
}

void copyValueAtIndex(lua_State *L1, lua_State *L2, int index) {
  if (lua_isnumber(L1, index)) {
    lua_pushnumber(L2, lua_tonumber(L1, index));
  } else if (lua_isboolean(L1, index)) {
    lua_pushboolean(L2, lua_toboolean(L1, index));
  } else if (lua_isnil(L1, index)) {
    lua_pushnil(L2);
  } else if (lua_isstring(L1, index)) {
    lua_pushstring(L2, lua_tostring(L1, index));
  } else if (lua_istable(L1, index)) {
    lua_newtable(L2);
    lua_pushnil(L1);
    while (lua_next(L1, index - (index < 0 ? 1 : 0)) != 0) {
      copyValueAtIndex(L1, L2, -2);
      copyValueAtIndex(L1, L2, -1);
      lua_settable(L2, -3);
      lua_pop(L1, 1);
    }
  } else {
    luaL_error(L1, "Can't copy type '%s' to send as stage event. %s",
               luaL_typename(L1, index),
               "Valid stage event types: nil, number, string, table, boolean.");
  }
}

void copyValue(lua_State *L1, lua_State *L2) {
  copyValueAtIndex(L1, L2, -1);
}

int Admin_sendEvent(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    Team *team = ship->properties->engine->getTeam(ship->teamIndex);
    lua_State *teamState = team->state;

    if (team->stageEventRef == 0) {
      lua_newtable(teamState);
      team->stageEventRef = luaL_ref(teamState, LUA_REGISTRYINDEX);
    }
    lua_rawgeti(teamState, LUA_REGISTRYINDEX, team->stageEventRef);
    int newIndex = (int) lua_objlen(L, -1) + 1;

    copyValue(L, teamState);
    lua_rawseti(teamState, -2, newIndex);
  }
  return 1;
}

int Admin_shipKills(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    lua_pushnumber(L, ship->kills);
  }
  return 1;
}

int Admin_shipDamage(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    lua_pushnumber(L, ship->damage);
  }
  return 1;
}

int Admin_shipFriendlyKills(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    lua_pushnumber(L, ship->friendlyKills);
  }
  return 1;
}

int Admin_shipFriendlyDamage(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = getShip(L, 2, admin->engine);
  if (ship != 0) {
    lua_pushnumber(L, ship->friendlyDamage);
  }
  return 1;
}

const char* getTeamName(lua_State *L, int index) {
  if (lua_isstring(L, index)) {
    return luaL_checkstring(L, index);
  } else if (lua_isuserdata(L, index)) {
    Ship *ship = checkShip(L, index);
    return ship->properties->engine->getTeam(ship->teamIndex)->name;
  } else {
    return 0;
  }
}

int Admin_setWinner(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  const char *teamName = getTeamName(L, 2);
  if (teamName != 0) {
    admin->engine->setWinnerName(teamName);
  }
  return 1;
}

int Admin_setRank(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  const char *teamName = getTeamName(L, 2);
  if (teamName != 0) {
    int rank = luaL_checkint(L, 3);
    admin->engine->setRank(teamName, rank);
  }
  return 1;
}

int Admin_setScore(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  const char *teamName = getTeamName(L, 2);
  if (teamName != 0) {
    double score = luaL_checknumber(L, 3);
    admin->engine->setScore(teamName, score);
  }
  return 1;
}

int Admin_setStatistic(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  const char *teamName = getTeamName(L, 2);
  if (teamName != 0) {
    const char *key = luaL_checkstring(L, 3);
    double value = luaL_checknumber(L, 4);
    admin->engine->setStatistic(teamName, key, value);
  }
  return 1;
}

int Admin_roundOver(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  BerryBotsEngine *engine = admin->engine;
  if (!engine->isRoundOver()) {
    engine->processRoundOver();
    engine->setRoundOver(true);
  }
  return 1;
}

int Admin_gameOver(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  BerryBotsEngine *engine = admin->engine;
  if (!engine->isGameOver()) {
    engine->processGameOver();
    engine->setGameOver(true);
  }
  return 1;
}

int Admin_drawText(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  BerryBotsEngine *engine = admin->engine;
  const char *text = luaL_checkstring(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  int fontSize = luaL_optint(L, 5, DEFAULT_STAGE_TEXT_SIZE);
  RgbaColor textColor;
  if (lua_istable(L, 6)) {
    textColor = getRgbaColor(L, 6);
  } else {
    textColor = getSolidWhiteColor();
  }
  int drawTicks = std::max(1, luaL_optint(L, 7, 1));
  admin->engine->getStage()->addStageText(engine->getGameTime(), text, x, y,
                                          fontSize, textColor, drawTicks);
  return 1;
}

const luaL_Reg Admin_methods[] = {
  {"destroyShip",             Admin_destroyShip},
  {"reviveShip",              Admin_reviveShip},
  {"moveShip",                Admin_moveShip},
  {"setShipSpeed",            Admin_setShipSpeed},
  {"setShipHeading",          Admin_setShipHeading},
  {"setShipEnergy",           Admin_setShipEnergy},
  {"setShipLaserEnabled",     Admin_setShipLaserEnabled},
  {"setShipTorpedoEnabled",   Admin_setShipTorpedoEnabled},
  {"setShipThrusterEnabled",  Admin_setShipThrusterEnabled},
  {"setShipEnergyEnabled",    Admin_setShipEnergyEnabled},
  {"setShipShowName",         Admin_setShipShowName},
  {"sendEvent",               Admin_sendEvent},
  {"shipKills",               Admin_shipKills},
  {"shipDamage",              Admin_shipDamage},
  {"shipFriendlyKills",       Admin_shipFriendlyKills},
  {"shipFriendlyDamage",      Admin_shipFriendlyDamage},
  {"setWinner",               Admin_setWinner},
  {"setRank",                 Admin_setRank},
  {"setScore",                Admin_setScore},
  {"setStatistic",            Admin_setStatistic},
  {"roundOver",               Admin_roundOver},
  {"gameOver",                Admin_gameOver},
  {"drawText",                Admin_drawText},
  {0, 0}
};

int registerAdmin(lua_State *L) {
  return registerClass(L, ADMIN, Admin_methods);
}

StageGfx* checkStageGfx(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  StageGfx *stageGfx = (StageGfx *) luaL_checkudata(L, index, STAGE_GFX);
  if (stageGfx == NULL) luaL_error(L, "error in checkStageGfx");
  return stageGfx;
}

StageGfx* pushStageGfx(lua_State *L) {
  StageGfx *stageGfx = (StageGfx *) lua_newuserdata(L, sizeof(StageGfx));
  luaL_getmetatable(L, STAGE_GFX);
  lua_setmetatable(L, -2);
  int stageGfxRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, stageGfxRef);
  return stageGfx;
}

int StageGfx_drawRectangle(lua_State *L) {
  StageGfx *stageGfx = checkStageGfx(L, 1);
  addUserGfxRectangle(L, stageGfx->engine, 0);
  return 1;
}

int StageGfx_drawLine(lua_State *L) {
  StageGfx *stageGfx = checkStageGfx(L, 1);
  addUserGfxLine(L, stageGfx->engine, 0);
  return 1;
}

int StageGfx_drawCircle(lua_State *L) {
  StageGfx *stageGfx = checkStageGfx(L, 1);
  addUserGfxCircle(L, stageGfx->engine, 0);
  return 1;
}

int StageGfx_drawText(lua_State *L) {
  StageGfx *stageGfx = checkStageGfx(L, 1);
  addUserGfxText(L, stageGfx->engine, 0);
  return 1;
}

int StageGfx_enabled(lua_State *L) {
  StageGfx *stageGfx = checkStageGfx(L, 1);
  lua_pushboolean(L, stageGfx->engine->getStage()->getGfxEnabled());
  return 1;
}

const luaL_Reg StageGfx_methods[] = {
  {"drawRectangle",  StageGfx_drawRectangle},
  {"drawLine",       StageGfx_drawLine},
  {"drawCircle",     StageGfx_drawCircle},
  {"drawText",       StageGfx_drawText},
  {"enabled",        StageGfx_enabled},
  {0, 0}
};

int registerStageGfx(lua_State *L) {
  return registerClass(L, STAGE_GFX, StageGfx_methods);
}

int ShipGlobals_print(lua_State *L) {
  int top = lua_gettop(L);
  const char *str = getPrintStr(L);
  BerryBotsEngine *engine = (BerryBotsEngine*) lua_getprinter(L);
  if (engine != 0) {
    engine->shipPrint(L, str);
  }
  return std::min(top, 1);
}

const luaL_Reg ShipGlobals_methods[] = {
  {"print",  ShipGlobals_print},
  {0, 0}
};

int registerShipGlobals(lua_State *L) {
  lua_getglobal(L, "_G");
  luaL_register(L, NULL, ShipGlobals_methods);
  lua_pop(L, 1);
  return 1;
}

int StageGlobals_print(lua_State *L) {
  int top = lua_gettop(L);
  const char *str = getPrintStr(L);
  BerryBotsEngine *engine = (BerryBotsEngine*) lua_getprinter(L);
  if (engine != 0) {
    engine->stagePrint(str);
  }
  return std::min(top, 1);
}

const luaL_Reg StageGlobals_methods[] = {
  {"print",  StageGlobals_print},
  {0, 0}
};

int registerStageGlobals(lua_State *L) {
  lua_getglobal(L, "_G");
  luaL_register(L, NULL, StageGlobals_methods);
  lua_pop(L, 1);
  return 1;
}

void getStringArgs(lua_State *L, int index, char** &strings, int &numStrings) {
  int top = lua_gettop(L);
  if (lua_istable(L, index)) {
    numStrings = (int) lua_objlen(L, index);
    strings = new char*[numStrings];
    int x = 0;
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
      const char *name = luaL_checkstring(L, -1);
      strings[x] = new char[strlen(name) + 1];
      strcpy(strings[x], name);
      lua_pop(L, 1);
      x++;
    }
  } else {
    numStrings = top - index + 1;
    strings = new char*[numStrings];
    for (int x = 0; x < numStrings; x++) {
      const char *name = luaL_checkstring(L, x + index);
      strings[x] = new char[strlen(name) + 1];
      strcpy(strings[x], name);
    }
  }
}

LuaRunnerForm* checkRunnerForm(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  LuaRunnerForm *form =
      (LuaRunnerForm *) luaL_checkudata(L, index, RUNNER_FORM);
  if (form == NULL) luaL_error(L, "error in checkRunnerForm");
  return form;
}

LuaRunnerForm* pushRunnerForm(lua_State *L, GameRunner *gameRunner) {
  LuaRunnerForm *form =
      (LuaRunnerForm *) lua_newuserdata(L, sizeof(LuaRunnerForm));
  luaL_getmetatable(L, RUNNER_FORM);
  lua_setmetatable(L, -2);
  int formRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, formRef);
  form->gameRunner = gameRunner;
  return form;
}

int RunnerForm_addStageSelect(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  form->gameRunner->addStageSelect(name);
  return 1;
}

int RunnerForm_addSingleShipSelect(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  form->gameRunner->addSingleShipSelect(name);
  return 1;
}

int RunnerForm_addMultiShipSelect(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  form->gameRunner->addMultiShipSelect(name);
  return 1;
}

int RunnerForm_addIntegerText(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  form->gameRunner->addIntegerText(name);
  return 1;
}

int RunnerForm_addCheckbox(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  form->gameRunner->addCheckbox(name);
  return 1;
}

int RunnerForm_addDropdown(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  int numOptions;
  char** options;
  getStringArgs(L, 3, options, numOptions);
  form->gameRunner->addDropdown(name, options, numOptions);
  delete options;
  return 1;
}

int RunnerForm_default(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  if (lua_isnumber(L, 3)) {
    form->gameRunner->setDefault(name, (int) luaL_checknumber(L, 3));
  } else if (lua_isboolean(L, 3)) {
    form->gameRunner->setDefault(name, (bool) lua_toboolean(L, 3));
  } else if (lua_isstring(L, 3) || lua_istable(L, 3)) {
    int numStrings;
    char** strings;
    getStringArgs(L, 3, strings, numStrings);
    for (int x = 0; x < numStrings; x++) {
      form->gameRunner->setDefault(name, strings[x]);
      delete strings[x];
    }
    delete strings;
  } else {
    luaL_error(L,
        "Second argument must be a number, string, or table of strings.");
  }
  return 1;
}

int RunnerForm_reset(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  form->gameRunner->reset();
  return 1;
}

int RunnerForm_ok(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *message = 0;
  if (lua_isstring(L, 2)) {
    message = lua_tostring(L, 2);
  }
  lua_pushboolean(L, form->gameRunner->ok(message));
  return 1;
}

int RunnerForm_get(lua_State *L) {
  LuaRunnerForm *form = checkRunnerForm(L, 1);
  const char *name = luaL_checkstring(L, 2);
  GameRunner *gameRunner = form->gameRunner;

  int type = gameRunner->getElementType(name);
  if (type == TYPE_MULTI_SHIP_SELECT) {
    char **stringValues = gameRunner->getStringValues(name);
    int numStringValues = gameRunner->getNumStringValues(name);
    lua_newtable(L);
    for (int x = 0; x < numStringValues; x++) {
      lua_pushstring(L, stringValues[x]);
      lua_rawseti(L, -2, x + 1);
    }
  } else if (type == TYPE_INTEGER_TEXT) {
    lua_pushnumber(L, gameRunner->getIntegerValue(name));
  } else if (type == TYPE_CHECKBOX) {
    lua_pushboolean(L, gameRunner->getBooleanValue(name));
  } else if (type == TYPE_STAGE_SELECT || type == TYPE_SINGLE_SHIP_SELECT
             || type == TYPE_DROPDOWN) {
    char **stringValues = gameRunner->getStringValues(name);
    int numStringValues = gameRunner->getNumStringValues(name);
    if (numStringValues == 0) {
      lua_pushliteral(L, "");
    } else {
      lua_pushstring(L, stringValues[0]);
    }
  } else if (type == RUNNER_UNDEFINED) {
    luaL_error(L, "Form name undefined: %s", name);
  }
  return 1;
}

const luaL_Reg RunnerForm_methods[] = {
  {"addStageSelect",       RunnerForm_addStageSelect},
  {"addSingleShipSelect",  RunnerForm_addSingleShipSelect},
  {"addMultiShipSelect",   RunnerForm_addMultiShipSelect},
  {"addIntegerText",       RunnerForm_addIntegerText},
  {"addCheckbox",          RunnerForm_addCheckbox},
  {"addDropdown",          RunnerForm_addDropdown},
  {"default",              RunnerForm_default},
  {"reset",                RunnerForm_reset},
  {"ok",                   RunnerForm_ok},
  {"get",                  RunnerForm_get},
  {0, 0}
};

int registerRunnerForm(lua_State *L) {
  return registerClass(L, RUNNER_FORM, RunnerForm_methods);
}

MatchRunner* checkGameRunner(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  MatchRunner *runner = (MatchRunner *) luaL_checkudata(L, index, MATCH_RUNNER);
  if (runner == NULL) luaL_error(L, "error in checkGameRunner");
  return runner;
}

MatchRunner* pushGameRunner(lua_State *L, GameRunner *gameRunner) {
  MatchRunner *luaRunner =
      (MatchRunner *) lua_newuserdata(L, sizeof(MatchRunner));
  luaL_getmetatable(L, MATCH_RUNNER);
  lua_setmetatable(L, -2);
  int runnerRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, runnerRef);
  luaRunner->gameRunner = gameRunner;
  luaRunner->replayBuilder = 0;
  return luaRunner;
}

int GameRunner_setThreadCount(lua_State *L) {
  MatchRunner *runner = checkGameRunner(L, 1);
  if (runner->gameRunner->started()) {
    luaL_error(L, "Can't set thread count after starting the first match.");
  } else {
    int threadCount = std::max(1, luaL_checkint(L, 2));
    runner->gameRunner->setThreadCount(threadCount);
  }
  return 1;
}

int GameRunner_queueMatch(lua_State *L) {
  MatchRunner *runner = checkGameRunner(L, 1);
  if (lua_gettop(L) < 3) {
    luaL_error(L, "Need at least one stage and one ship to queue a match.");
  } else {
    const char *stageName = luaL_checkstring(L, 2);
    int numShips;
    char **ships;
    getStringArgs(L, 3, ships, numShips);
    runner->gameRunner->queueMatch(stageName, ships, numShips);
    for (int x = 0; x < numShips; x++) {
      delete ships[x];
    }
    delete ships;
  }
  return 1;
}

int GameRunner_empty(lua_State *L) {
  MatchRunner *runner = checkGameRunner(L, 1);
  lua_pushboolean(L, runner->gameRunner->empty());
  return 1;
}

int GameRunner_nextResult(lua_State *L) {
  MatchRunner *runner = checkGameRunner(L, 1);
  MatchResult *result = runner->gameRunner->nextResult();
  if (runner->replayBuilder != 0) {
    runner->gameRunner->deleteReplayBuilder(runner->replayBuilder);
    runner->replayBuilder = 0;
  }
  if (result == 0) {
    lua_pushnil(L);
  } else {
    runner->replayBuilder = result->getReplayBuilder();
    bool hasScores = result->hasScores();
    lua_newtable(L);
    setField(L, "stage", result->getStageName());
    if (result->errored()) {
      setField(L, "errored", true);
      setField(L, "errorMessage", result->getErrorMessage());
    } else {
      setField(L, "errored", false);
      setFieldNil(L, "errorMessage");
      const char *winner = result->getWinner();
      if (winner == 0) {
        setFieldNil(L, "winner");
      } else {
        setField(L, "winner", winner);
      }
      lua_pushstring(L, "teams");
      lua_newtable(L);
      char **teamNames = result->getTeamNames();
      TeamResult **teamResults = result->getTeamResults();
      int numTeams = result->getNumTeams();
      int resultIndex = 0;
      for (int x = 0; x < numTeams; x++) {
        TeamResult *teamResult = teamResults[x];
        if (teamResult->showResult) {
          lua_newtable(L);
          setField(L, "name", teamNames[x]);
          setField(L, "rank", teamResult->rank);
          if (hasScores) {
            setField(L, "score", teamResult->score);
          } else {
            setFieldNil(L, "score");
          }
          if (teamResult->numStats == 0) {
            setFieldNil(L, "stats");
          } else {
            lua_pushliteral(L, "stats");
            lua_newtable(L);
            for (int y = 0; y < teamResult->numStats; y++) {
              setField(L, teamResult->stats[y]->key,
                       teamResult->stats[y]->value);
            }
            lua_settable(L, -3);
          }
          lua_rawseti(L, -2, ++resultIndex);
        }
      }
      lua_settable(L, -3);
    }
    delete result;
  }
  return 1;
}

char* newFilename(const char *stageName, const char *timestamp) {
  std::stringstream nameStream;
  nameStream << ((stageName == 0) ? "unknown" : stageName) << "-" << timestamp
             << "-";
  nameStream << std::hex << (rand() % 4096);
  nameStream << ".html";

  std::string filename = nameStream.str();
  char *newFilename = new char[filename.length() + 1];
  strcpy(newFilename, filename.c_str());
  
  return newFilename;
}

int GameRunner_saveReplay(lua_State *L) {
  MatchRunner *runner = checkGameRunner(L, 1);
  if (runner->replayBuilder == 0) {
    lua_pushnil(L);
  } else {
    FileManager *fileManager = new FileManager();
    char *absFilename = 0;
    do {
      if (absFilename != 0) {
        delete absFilename;
      }
      char *timestamp = getTimestamp();
      const char *filename =
          newFilename(runner->replayBuilder->getStageName(), timestamp);
      absFilename = fileManager->getFilePath(getReplaysDir().c_str(), filename);
      runner->replayBuilder->setTimestamp(timestamp);
      delete timestamp;
    } while (fileManager->fileExists(absFilename));

    if (absFilename != 0) {
      runner->replayBuilder->saveReplay(absFilename);
      lua_pushstring(L, absFilename);
      delete absFilename;
    }
    delete fileManager;
  }
  return 1;
}

const luaL_Reg GameRunner_methods[] = {
  {"setThreadCount",  GameRunner_setThreadCount},
  {"queueMatch",      GameRunner_queueMatch},
  {"empty",           GameRunner_empty},
  {"nextResult",      GameRunner_nextResult},
  {"saveReplay",      GameRunner_saveReplay},
  {0, 0}
};

int registerGameRunner(lua_State *L) {
  return registerClass(L, MATCH_RUNNER, GameRunner_methods);
}

RunnerFiles* checkRunnerFiles(lua_State *L, int index) {
  luaL_checktype(L, index, LUA_TUSERDATA);
  RunnerFiles *files = (RunnerFiles *) luaL_checkudata(L, index, RUNNER_FILES);
  if (files == NULL) luaL_error(L, "error in checkRunnerFiles");
  return files;
}

RunnerFiles* pushRunnerFiles(lua_State *L, GameRunner *gameRunner) {
  RunnerFiles *files = (RunnerFiles *) lua_newuserdata(L, sizeof(RunnerFiles));
  luaL_getmetatable(L, RUNNER_FILES);
  lua_setmetatable(L, -2);
  int filesRef = luaL_ref(L, LUA_REGISTRYINDEX); // to keep it from GC
  lua_rawgeti(L, LUA_REGISTRYINDEX, filesRef);
  files->gameRunner = gameRunner;
  return files;
}

char* checkFilename(lua_State *L, int index, const char *dir) {
  const char *rawFilename = luaL_checkstring(L, index);
  char *filename = new char[strlen(rawFilename) + 1];
  strcpy(filename, rawFilename);
  FileManager *fileManager = new FileManager();
  fileManager->fixSlashes(filename);
  if (fileManager->isAbsPath(filename)) {
    delete filename;
    delete fileManager;
    luaL_error(L, "Can't read from absolute paths.");
    return 0;
  } else {
    char *absFilename = fileManager->getFilePath(dir, filename);
    bool error = false;
    if (strncmp(dir, absFilename, strlen(dir))) {
      error = true;
    }
    delete filename;
    if (error) {
      delete absFilename;
      delete fileManager;
      luaL_error(L, "Can only read from below runners directory.");
      return 0;
    } else {
      delete fileManager;
      return absFilename;
    }
  }
}

char* checkFilename(lua_State *L, int index) {
  return checkFilename(L, index, getRunnersDir().c_str());
}

char* checkBotFilename(lua_State *L, int index) {
  std::string runnerBotsDir(getShipsDir());
  runnerBotsDir.append(BB_DIRSEP);
  runnerBotsDir.append("runners");
  return checkFilename(L, index, runnerBotsDir.c_str());
}

char* checkStageFilename(lua_State *L, int index) {
  std::string runnerStagesDir(getStagesDir());
  runnerStagesDir.append(BB_DIRSEP);
  runnerStagesDir.append("runners");
  return checkFilename(L, index, runnerStagesDir.c_str());
}

void fileExists(lua_State *L, const char *filename) {
  FileManager *fileManager = new FileManager();
  lua_pushboolean(L, fileManager->fileExists(filename));
  delete filename;
  delete fileManager;
}

void readFile(lua_State *L, const char *filename) {
  FileManager *fileManager = new FileManager();
  if (fileManager->fileExists(filename)) {
    try {
      char *fileContents = fileManager->readFile(filename);
      lua_pushstring(L, fileContents);
      delete fileContents;
    } catch (FileNotFoundException *e) {
      delete e;
      lua_pushnil(L);
    }
  } else {
    lua_pushnil(L);
  }
  delete filename;
  delete fileManager;
}

void writeFile(lua_State *L, const char *filename) {
  if (lua_isstring(L, 3)) {
    FileManager *fileManager = new FileManager();
    fileManager->writeFile(filename, lua_tostring(L, 3));
    delete filename;
    delete fileManager;
  } else {
    delete filename;
    luaL_error(L, "No file contents.");
  }
}

int RunnerFiles_exists(lua_State *L) {
  checkRunnerFiles(L, 1);
  fileExists(L, checkFilename(L, 2));
  return 1;
}

int RunnerFiles_read(lua_State *L) {
  readFile(L, checkFilename(L, 2));
  return 1;
}

int RunnerFiles_write(lua_State *L) {
  writeFile(L, checkFilename(L, 2));
  return 1;
}

int RunnerFiles_botExists(lua_State *L) {
  checkRunnerFiles(L, 1);
  fileExists(L, checkBotFilename(L, 2));
  return 1;
}

int RunnerFiles_readBot(lua_State *L) {
  readFile(L, checkBotFilename(L, 2));
  return 1;
}

int RunnerFiles_writeBot(lua_State *L) {
  writeFile(L, checkBotFilename(L, 2));
  return 1;
}

int RunnerFiles_stageExists(lua_State *L) {
  checkRunnerFiles(L, 1);
  fileExists(L, checkStageFilename(L, 2));
  return 1;
}

int RunnerFiles_readStage(lua_State *L) {
  readFile(L, checkStageFilename(L, 2));
  return 1;
}

int RunnerFiles_writeStage(lua_State *L) {
  writeFile(L, checkStageFilename(L, 2));
  return 1;
}

const luaL_Reg RunnerFiles_methods[] = {
  {"exists",       RunnerFiles_exists},
  {"read",         RunnerFiles_read},
  {"write",        RunnerFiles_write},
  {"botExists",    RunnerFiles_botExists},
  {"readBot",      RunnerFiles_readBot},
  {"writeBot",     RunnerFiles_writeBot},
  {"stageExists",  RunnerFiles_stageExists},
  {"readStage",    RunnerFiles_readStage},
  {"writeStage",   RunnerFiles_writeStage},
  {0, 0}
};

int registerRunnerFiles(lua_State *L) {
  return registerClass(L, RUNNER_FILES, RunnerFiles_methods);
}

int RunnerGlobals_print(lua_State *L) {
  int top = lua_gettop(L);
  const char *str = getPrintStr(L);
  PrintHandler *printHandler = (PrintHandler*) lua_getprinter(L);
  if (printHandler != 0) {
    printHandler->runnerPrint(str);
  }
  return std::min(top, 1);
}

const luaL_Reg RunnerGlobals_methods[] = {
  {"print",  RunnerGlobals_print},
  {0, 0}
};

int registerRunnerGlobals(lua_State *L) {
  lua_getglobal(L, "_G");
  luaL_register(L, NULL, RunnerGlobals_methods);
  lua_pop(L, 1);
  return 1;
}
