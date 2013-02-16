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

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include "stage.h"
#include "sensorhandler.h"
#include "bbengine.h"
#include "bblua.h"
#include "printhandler.h"

extern PrintHandler *printHandler;

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
  registerShipGlobals(*shipState);
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
  lua_pop(L, 1);
  return 1;
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
  if (!engine->isShipInitComplete()) {
    strncpy(ship->properties->name, shipName, MAX_NAME_LENGTH);
    ship->properties->name[MAX_NAME_LENGTH] = '\0';
    Team *team = engine->getTeam(ship->teamIndex);
    if (team->numShips == 1) {
      setTeamName(team, shipName);
    }
    if (printHandler != 0) {
      std::stringstream ss;
      if (team->numShips == 1) {
        ss << "Set";
      } else {
        ss << "Ship " << (ship->index - team->firstShipIndex + 1) << " set";
      }
      ss << " name: " << shipName;
      printHandler->shipPrint(L, ss.str().c_str());
    }
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setTeamName(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  const char *teamName = luaL_checkstring(L, 2);
  BerryBotsEngine *engine = ship->properties->engine;
  if (!engine->isShipInitComplete()) {
    Team *team = engine->getTeam(ship->teamIndex);
    setTeamName(team, teamName);
    if (printHandler != 0) {
      std::stringstream ss;
      ss << "Set team name: " << teamName;
      printHandler->shipPrint(L, ss.str().c_str());
    }
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setShipColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (!ship->properties->engine->isShipInitComplete()) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->shipR = r;
    ship->properties->shipG = g;
    ship->properties->shipB = b;
    Team *team = ship->properties->engine->getTeam(ship->teamIndex);
    if (printHandler != 0) {
      std::stringstream ss;
      if (team->numShips == 1) {
        ss << "Set";
      } else {
        ss << "Ship " << (ship->index - team->firstShipIndex + 1) << " set";
      }
      ss << " ship color: (" << r << ", " << g << ", " << b << ")";
      printHandler->shipPrint(L, ss.str().c_str());
    }
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setLaserColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (!ship->properties->engine->isShipInitComplete()) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->laserR = r;
    ship->properties->laserG = g;
    ship->properties->laserB = b;
    Team *team = ship->properties->engine->getTeam(ship->teamIndex);
    if (printHandler != 0) {
      std::stringstream ss;
      if (team->numShips == 1) {
        ss << "Set";
      } else {
        ss << "Ship " << (ship->index - team->firstShipIndex + 1) << " set";
      }
      ss << " laser color: (" << r << ", " << g << ", " << b << ")";
      printHandler->shipPrint(L, ss.str().c_str());
    }
  }
  lua_settop(L, 1);
  return 1;
}

int Ship_setThrusterColor(lua_State *L) {
  Ship *ship = checkShip(L, 1);
  if (!ship->properties->engine->isShipInitComplete()) {
    int r = limit(0, luaL_checkint(L, 2), 255);
    int g = limit(0, luaL_checkint(L, 3), 255);
    int b = limit(0, luaL_checkint(L, 4), 255);
    ship->properties->thrusterR = r;
    ship->properties->thrusterG = g;
    ship->properties->thrusterB = b;
    Team *team = ship->properties->engine->getTeam(ship->teamIndex);
    if (printHandler != 0) {
      std::stringstream ss;
      if (team->numShips == 1) {
        ss << "Set";
      } else {
        ss << "Ship " << (ship->index - team->firstShipIndex + 1) << " set";
      }
      ss << " thruster color: (" << r << ", " << g << ", " << b << ")";
      printHandler->shipPrint(L, ss.str().c_str());
    }
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
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set stage size outside of 'configure' function.");
  } else {
    stageBuilder->engine->getStage()->setSize(width, height);
    if (printHandler != 0) {
      std::stringstream ss;
      ss << "Set stage size: " << width << " x " << height;
      printHandler->stagePrint(ss.str().c_str());
    }
  }
  return 1;
}

int StageBuilder_setBattleMode(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  bool battleMode = lua_toboolean(L, 2);
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set battle mode outside of 'configure' function.");
  } else {
    stageBuilder->engine->setBattleMode(battleMode);
    if (printHandler != 0) {
      std::stringstream ss;
      ss << "Set battle mode: " << (battleMode ? "true" : "false");
      printHandler->stagePrint(ss.str().c_str());
    }
  }
  return 1;
}

int StageBuilder_addWall(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int left = luaL_checkint(L, 2);
  int bottom = luaL_checkint(L, 3);
  int width = luaL_checkint(L, 4);
  int height = luaL_checkint(L, 5);
  Stage *stage = stageBuilder->engine->getStage();
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add walls outside of 'configure' function.");
  } else if (!stage->addWall(left, bottom, width, height, true)) {
    luaL_error(L, "Failed to add wall - is %i too many?",
               stage->getWallCount());
  }
  if (printHandler != 0) {
    std::stringstream ss;
    ss << "Added wall: bottom left (" << left << ", " << bottom << "), "
       << "dimensions " << width << " x " << height;
    printHandler->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_addStart(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  Stage *stage = stageBuilder->engine->getStage();
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add starts outside of 'configure' function.");
  } else if (!stage->addStart(x, y)) {
    luaL_error(L, "Failed to add start - is %i too many?",
               stage->getStartCount());
  }
  if (printHandler != 0) {
    std::stringstream ss;
    ss << "Added start position: (" << x << ", " << y << ")";
    printHandler->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_addZone(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int left = luaL_checkint(L, 2);
  int bottom = luaL_checkint(L, 3);
  int width = luaL_checkint(L, 4);
  int height = luaL_checkint(L, 5);
  const char *zoneTag = luaL_optstring(L, 6, "");
  Stage *stage = stageBuilder->engine->getStage();
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add zones outside of 'configure' function.");
  } else if (!stage->addZone(left, bottom, width, height, zoneTag)) {
    luaL_error(L, "Failed to add zone - is %i too many?",
               stage->getZoneCount());
  }
  if (printHandler != 0) {
    std::stringstream ss;
    ss << "Added zone: bottom left (" << left << ", " << bottom << "), "
       << "dimensions: " << width << " x " << height;
    printHandler->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_addShip(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  const char *stageShipFilename = luaL_checkstring(L, 2);
  Stage *stage = stageBuilder->engine->getStage();
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't add stage ships outside of 'configure' function.");
  } else if (!stage->addStageShip(stageShipFilename)) {
    luaL_error(L, "Failed to add stage ship - is %i too many?",
               stage->getStageShipCount());
  }
  if (printHandler != 0) {
    std::stringstream ss;
    ss << "Added stage ship: " << stageShipFilename;
    printHandler->stagePrint(ss.str().c_str());
  }
  return 1;
}

int StageBuilder_setTeamSize(lua_State *L) {
  StageBuilder *stageBuilder = checkStageBuilder(L, 1);
  int teamSize = luaL_checkint(L, 2);
  if (stageBuilder->engine->isStageConfigureComplete()) {
    luaL_error(L, "Can't set team size outside of 'configure' function.");
  } else {
    stageBuilder->engine->setTeamSize(teamSize);
    if (printHandler != 0) {
      std::stringstream ss;
      ss << "Set team size: " << teamSize;
      printHandler->stagePrint(ss.str().c_str());
    }
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

int Admin_destroyShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  admin->engine->destroyShip(ship);
  return 1;
}

int Admin_reviveShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->alive = true;
  ship->energy = DEFAULT_ENERGY;
  admin->engine->getStage()->updateShipPosition(ship, ship->x, ship->y);
  return 1;
}

int Admin_moveShip(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  admin->engine->getStage()->updateShipPosition(ship, x, y);
  return 1;
}

int Admin_setShipSpeed(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->speed = luaL_checknumber(L, 3);
  return 1;
}

int Admin_setShipHeading(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->heading = luaL_checknumber(L, 3);
  return 1;
}

int Admin_setShipEnergy(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->energy = luaL_checknumber(L, 3);
  return 1;
}

int Admin_setShipLaserEnabled(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->laserEnabled = lua_toboolean(L, 3);
  return 1;
}

int Admin_setShipTorpedoEnabled(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->torpedoEnabled = lua_toboolean(L, 3);
  return 1;
}

int Admin_setShipThrusterEnabled(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->thrusterEnabled = lua_toboolean(L, 3);
  return 1;
}

int Admin_setShipEnergyEnabled(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->energyEnabled = lua_toboolean(L, 3);
  return 1;
}

int Admin_setShipShowName(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  ship->showName = lua_toboolean(L, 3);
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
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
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

  return 1;
}

int Admin_shipKills(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushnumber(L, ship->kills);
  return 1;
}

int Admin_shipDamage(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushnumber(L, ship->damage);
  return 1;
}

int Admin_shipFriendlyKills(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushnumber(L, ship->friendlyKills);
  return 1;
}

int Admin_shipFriendlyDamage(lua_State *L) {
  checkAdmin(L, 1);
  Ship *ship = checkShip(L, 2);
  lua_pushnumber(L, ship->friendlyDamage);
  return 1;
}

int Admin_setWinner(lua_State *L) {
  Admin *admin = checkAdmin(L, 1);
  const char *winnerName = luaL_checkstring(L, 2);
  admin->engine->setWinnerName(winnerName);
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
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  const char *text = luaL_checkstring(L, 4);
  admin->engine->getStage()->addText(x, y, text);
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
  {"roundOver",               Admin_roundOver},
  {"gameOver",                Admin_gameOver},
  {"drawText",                Admin_drawText},
  {0, 0}
};

int registerAdmin(lua_State *L) {
  return registerClass(L, ADMIN, Admin_methods);
}

int ShipGlobals_print(lua_State *L) {
  int top = lua_gettop(L);
  const char *str = luaL_optstring(L, 1, "");
  if (printHandler != 0) {
    printHandler->shipPrint(L, str);
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
  const char *str = luaL_optstring(L, 1, "");
  if (printHandler != 0) {
    printHandler->stagePrint(str);
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
