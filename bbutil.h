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

#ifndef BBUTIL_H
#define BBUTIL_H

#include <platformstl/performance/performance_counter.hpp>
#include "bbconst.h"

extern "C" {
  #include "lua.h"
}

#define SHIP           "Ship"
#define SENSORS        "Sensors"
#define STAGE_BUILDER  "StageBuilder"
#define WALL           "Wall"
#define ZONE           "Zone"
#define WORLD          "World"
#define SHIP_GFX       "ShipGfx"
#define ADMIN          "Admin"
#define STAGE_SENSORS  "StageSensors"
#define STAGE_GFX      "StageGfx"
#define RUNNER_FORM    "RunnerForm"
#define GAME_RUNNER    "LuaGameRunner"
#define RUNNER_FILES   "RunnerFiles"

class BerryBotsEngine;
class GameRunner;

// Graphic definition structs

typedef struct {
  int r;
  int g;
  int b;
  int a;
} RgbaColor;

typedef struct {
  char *text;
  double x;
  double y;
  int fontSize;
  int startTime;
  int drawTicks;
} StageText;

typedef struct {
  double left;
  double bottom;
  double width;
  double height;
  double rotation;
  short fillR;
  short fillG;
  short fillB;
  short fillA;
  double outlineThickness;
  short outlineR;
  short outlineG;
  short outlineB;
  short outlineA;
  int startTime;
  int drawTicks;
} UserGfxRectangle;

typedef struct {
  double x;
  double y;
  double angle;
  double length;
  double thickness;
  short fillR;
  short fillG;
  short fillB;
  short fillA;
  double outlineThickness;
  short outlineR;
  short outlineG;
  short outlineB;
  short outlineA;
  int startTime;
  int drawTicks;
} UserGfxLine;

typedef struct {
  double x;
  double y;
  double radius;
  short fillR;
  short fillG;
  short fillB;
  short fillA;
  double outlineThickness;
  short outlineR;
  short outlineG;
  short outlineB;
  short outlineA;
  int startTime;
  int drawTicks;
} UserGfxCircle;

typedef struct {
  char *text;
  double x;
  double y;
  int fontSize;
  short textR;
  short textG;
  short textB;
  short textA;
  int startTime;
  int drawTicks;
} UserGfxText;


// Game engine structs

typedef struct {
  short index;
  short firstShipIndex;
  short numShips;
  short shipsAlive;
  bool hasRoundOver;
  bool hasGameOver;
  int stageEventRef;
  lua_State *state;
  char name[MAX_NAME_LENGTH + 1];
  platformstl::performance_counter counter;
  unsigned long long cpuTime[CPU_TIME_TICKS];
  unsigned long long totalCpuTime;
  unsigned int totalCpuTicks;
  bool stageShip;
  bool disabled;
  bool errored;
  bool ownedByLua;
  bool gfxEnabled;
  UserGfxRectangle* gfxRectangles[MAX_USER_RECTANGLES];
  int numRectangles;
  bool tooManyRectangles;
  UserGfxLine* gfxLines[MAX_USER_LINES];
  int numLines;
  bool tooManyLines;
  UserGfxCircle* gfxCircles[MAX_USER_RECTANGLES];
  int numCircles;
  bool tooManyCircles;
  UserGfxText* gfxTexts[MAX_USER_TEXTS];
  int numTexts;
  bool tooManyTexts;
} Team;

// Static ship properties that we don't need to copy around.
typedef struct {
  char name[MAX_NAME_LENGTH + 1];
  short shipR;
  short shipG;
  short shipB;
  short laserR;
  short laserG;
  short laserB;
  short thrusterR;
  short thrusterG;
  short thrusterB;
  bool stageShip;
  bool disabled;
  bool ownedByLua;
  BerryBotsEngine *engine;
} ShipProperties;

typedef struct {
  short index;
  short teamIndex;
  double thrusterAngle;
  double thrusterForce;
  double x;
  double y;
  double heading;
  double speed;
  double energy;
  short laserGunHeat;
  short torpedoGunHeat;
  bool hitWall;
  bool hitShip;
  bool alive;
  bool laserEnabled;
  bool torpedoEnabled;
  bool thrusterEnabled;
  bool energyEnabled;
  bool showName;
  double kills;
  double friendlyKills;
  double damage;
  double friendlyDamage;
  ShipProperties *properties;
} Ship;

typedef struct {
  int hitByShipRef;
  int hitByLaserRef;
  int hitByTorpedoRef;
  int hitWallRef;
  int shipDestroyedRef;
  int shipFiredLaserRef;
  int shipFiredTorpedoRef;
  int laserHitShipRef;
  int stageEventRef;
} Sensors;

typedef struct {
  short shipIndex;
  int fireTime;
  double x;
  double y;
  double heading;
  double dx;
  double dy;
  bool dead;
} Laser;

typedef struct {
  short shipIndex;
  double x;
  double y;
  double heading;
  double distance;
  double dx;
  double dy;
  double distanceTraveled;
} Torpedo;

typedef struct {
  BerryBotsEngine *engine;
} StageBuilder;

typedef struct {
  int width;
  int height;
  short numShips;
  short teamSize;
  int time;
  int wallsRef;
  int zonesRef;
  int constantsRef;
  BerryBotsEngine *engine;
} World;

typedef struct {
  Team *team;
  BerryBotsEngine *engine;
} ShipGfx;

typedef struct {
  BerryBotsEngine *engine;
} Admin;

typedef struct {
  BerryBotsEngine *engine;
} StageGfx;

typedef struct {
  int shipHitShipRef;
  int laserHitShipRef;
  int torpedoHitShipRef;
  int shipHitWallRef;
  int shipDestroyedRef;
  int shipFiredLaserRef;
  int shipFiredTorpedoRef;
} StageSensors;

typedef struct {
  GameRunner *gameRunner;
} RunnerForm;

typedef struct {
  GameRunner *gameRunner;
} LuaGameRunner;

typedef struct {
  GameRunner *gameRunner;
} RunnerFiles;

extern double limit(double p, double q, double r);
extern int signum(double x);
extern double square(double x);
extern double abs(double x);
extern double round(double d, int x);
extern double normalRelativeAngle(double x);
extern double normalAbsoluteAngle(double x);
extern double toDegrees(double x);
extern char** parseFlag(
    int argc, char *argv[], const char *flag, int numValues);
extern bool flagExists(int argc, char *argv[], const char *flag);
extern bool isWhitespace(const char *s);

#endif
