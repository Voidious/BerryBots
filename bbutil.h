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
#define ADMIN          "Admin"
#define STAGE_SENSORS  "StageSensors"

typedef struct {
  unsigned short index;
  unsigned short firstShipIndex;
  unsigned short numShips;
  unsigned short shipsAlive;
  bool hasRoundOver;
  bool hasGameOver;
  int stageEventRef;
  lua_State *state;
  char name[MAX_NAME_LENGTH + 1];
  platformstl::performance_counter counter;
  unsigned long long cpuTime[CPU_TIME_TICKS];
  unsigned long long totalCpuTime;
  unsigned int totalCpuTicks;
  bool doa;
  bool stageShip;
} Team;

typedef struct {
  char name[MAX_NAME_LENGTH + 1];
  unsigned short shipR;
  unsigned short shipG;
  unsigned short shipB;
  unsigned short laserR;
  unsigned short laserG;
  unsigned short laserB;
  unsigned short thrusterR;
  unsigned short thrusterG;
  unsigned short thrusterB;
  bool doa;
  bool stageShip;
} ShipProperties;

typedef struct {
  unsigned short index;
  unsigned short teamIndex;
  double thrusterAngle;
  double thrusterForce;
  double x;
  double y;
  double heading;
  double speed;
  double energy;
  unsigned char laserGunHeat;
  unsigned char torpedoGunHeat;
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
  unsigned short shipIndex;
  double x;
  double y;
  double heading;
  double dx;
  double dy;
  bool dead;
} Laser;

typedef struct {
  unsigned short shipIndex;
  double x;
  double y;
  double heading;
  double distance;
  double dx;
  double dy;
  double distanceTraveled;
} Torpedo;

typedef struct {
  unsigned short width;
  unsigned short height;
  unsigned short numShips;
  unsigned short teamSize;
  int time;
  int wallsRef;
  int zonesRef;
  int constantsRef;
} World;

typedef struct {
  int shipHitShipRef;
  int laserHitShipRef;
  int torpedoHitShipRef;
  int shipHitWallRef;
  int shipDestroyedRef;
  int shipFiredLaserRef;
  int shipFiredTorpedoRef;
} StageSensors;

extern int min(int p, int q);
extern double min(double p, double q);
extern double max(double p, double q);
extern double limit(double p, double q, double r);
extern int signum(double x);
extern double square(double x);
extern double abs(double x);
extern double normalRelativeAngle(double x);
extern double normalAbsoluteAngle(double x);
extern double toDegrees(double x);
extern char** parseFlag(
    int argc, char *argv[], const char *flag, int numValues);
extern bool flagExists(int argc, char *argv[], const char *flag);
extern void loadStageFile(const char *filename, char **stageDir,
    char **stageDirFilename, char **stageCwd, const char *cacheDir);
extern void loadBotFile(const char *filename, char **botDir, char **botDirFilename,
    char **botCwd, const char *cacheDir);
extern bool isLuaFilename(const char *filename);
extern bool isZipFilename(const char *filename);
extern void checkLuaFilename(const char *filename);
extern void packageStage(const char *stageArg, const char *version,
                         const char *cacheDir, const char *tmpDir, bool nosrc);
extern void packageBot(char *botArg, char *version, const char *cacheDir,
    const char *tmpDir, bool nosrc);

#endif
