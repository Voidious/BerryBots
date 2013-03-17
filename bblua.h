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

#ifndef BBLUA_H
#define BBLUA_H

#include <exception>
#include "bbutil.h"
#include "stage.h"
#include "sensorhandler.h"

// Time limit allotted to Lua calls, in microseconds. These are hard limits to
// deal with misbehaving user code. CPU usage limits for fairness and
// competitive reasons will be added later, and will be much more robust.
#ifdef __ARM_ARCH_6__     // Raspberry Pi
#define PCALL_TIME_LIMIT  10000000 // 10.0s
#else
#define PCALL_TIME_LIMIT    500000 //  0.5s
#endif

#define DEFAULT_STAGE_TEXT_SIZE    28
#define DEFAULT_SHIP_TEXT_SIZE     28
#define TRANSPARENT_COLOR          {0, 0, 0, 0}
#define SOLID_WHITE_COLOR          {255, 255, 255, 255}
#define DEFAULT_OUTLINE_THICKNESS  2

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

extern int registerShip(lua_State *L);
extern int registerSensors(lua_State *L);
extern int registerStageSensors(lua_State *L);
extern int registerStageBuilder(lua_State *L);
extern int registerWall(lua_State *L);
extern int registerZone(lua_State *L);
extern int registerWorld(lua_State *L);
extern int registerShipGfx(lua_State *L);
extern int registerAdmin(lua_State *L);
extern int registerStageGfx(lua_State *L);
extern int registerShipGlobals(lua_State *L);
extern int registerStageGlobals(lua_State *L);

extern void killHook(lua_State *L, lua_Debug *ar);
extern void initStageState(lua_State **stageState, const char *stageCwd);
extern void initShipState(lua_State **shipState, const char *shipCwd);
extern Ship* pushShip(lua_State *L);
extern void pushVisibleEnemyShips(
    lua_State *L, bool *teamVision, int teamIndex, Ship **ships, int numShips);
extern Sensors* pushSensors(
    Team *team, SensorHandler *sensorHandler, ShipProperties **properties);
extern void cleanupSensorsTables(lua_State *L, Sensors *sensors);
extern StageSensors* pushStageSensors(lua_State *L,
    SensorHandler *sensorHandler, Ship **ships, ShipProperties **properties);
extern void cleanupStageSensorsTables(
    lua_State *L, StageSensors *stageSensors);
extern StageBuilder* pushStageBuilder(lua_State *L);
extern void pushWalls(lua_State *L, Wall** walls, int wallCount);
extern void pushZones(lua_State *L, Zone** zones, int zoneCount);
extern void pushCopyOfShips(
    lua_State *L, Ship** ships, Ship **shipsCopy, int numShips);
extern World* pushWorld(lua_State *L, Stage *stage, int numShips, int teamSize);
extern ShipGfx* pushShipGfx(lua_State *L);
extern Admin* pushAdmin(lua_State *L);
extern StageGfx* pushStageGfx(lua_State *L);
extern void crawlFiles(lua_State *L, const char *startFile);

#endif
