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

#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include "bbutil.h"
#include "eventhandler.h"

#define MAX_HIT_BY_SHIPS         32
#define MAX_HIT_BY_LASERS        1024
#define MAX_HIT_BY_TORPEDOS      1024
#define MAX_HIT_WALLS            4
#define MAX_SHIP_DESTROYEDS      1024
#define MAX_SHIP_HIT_SHIPS       4096
#define MAX_LASER_HIT_SHIPS      4096
#define MAX_TORPEDO_HIT_SHIPS    4096
#define MAX_SHIP_HIT_WALLS       4096
#define MAX_SHIP_FIRED_LASERS    2048
#define MAX_SHIP_FIRED_TORPEDOS  2048

typedef struct {
  int time;
  short targetShipIndex;
  double targetX;
  double targetY;
  short hittingShipIndex;
  double hittingX;
  double hittingY;
  double inAngle;
  double inForce;
  double outAngle;
  double outForce;
} HitByShip;

typedef struct {
  int time;
  short targetShipIndex;
  double laserX;
  double laserY;
  double laserHeading;
} HitByLaser;

typedef struct {
  int time;
  short targetShipIndex;
  double hitAngle;
  double hitForce;
  double hitDamage;
} HitByTorpedo;

typedef struct {
  int time;
  short shipIndex;
  double shipX;
  double shipY;
  double bounceAngle;
  double bounceForce;
} ShipHitWall;

typedef struct {
  int time;
  short shipIndex;
} ShipDestroyed;

typedef struct {
  int time;
  short shipIndex;
  double shipX;
  double shipY;
} ShipFiredLaser;

typedef struct {
  int time;
  short shipIndex;
  double shipX;
  double shipY;
} ShipFiredTorpedo;

typedef struct {
  int time;
  short targetShipIndex;
  double shipX;
  double shipY;
} LaserHitShip;

typedef struct {
  int time;
  short hittingShipIndex;
  double hittingX;
  double hittingY;
  short targetShipIndex;
  double targetX;
  double targetY;
  double inAngle;
  double inForce;
  double outAngle;
  double outForce;
} ShipHitShip;

typedef struct {
  int time;
  short shipIndex;
  double shipX;
  double shipY;
  double laserHeading;
} StageShipFiredLaser;

typedef struct {
  int time;
  short shipIndex;
  double shipX;
  double shipY;
  double torpedoHeading;
  double torpedoDistance;
} StageShipFiredTorpedo;

typedef struct {
  int time;
  short srcShipIndex;
  short targetShipIndex;
  double laserX;
  double laserY;
  double laserHeading;
} StageLaserHitShip;

typedef struct {
  int time;
  short srcShipIndex;
  short targetShipIndex;
  double hitAngle;
  double hitForce;
  double hitDamage;
} StageTorpedoHitShip;

class SensorHandler : public EventHandler {
  Team **teams_;
  int numTeams_;
  bool** teamVision_;

  // Events for the ships.
  HitByShip*** hitByShips_;
  int* numHitByShips_;
  HitByLaser*** hitByLasers_;
  int *numHitByLasers_;
  HitByTorpedo*** hitByTorpedos_;
  int *numHitByTorpedos_;
  ShipHitWall*** hitWalls_;
  int *numHitWalls_;
  ShipDestroyed*** shipDestroyeds_;
  int *numShipDestroyeds_;
  ShipFiredLaser*** shipFiredLasers_;
  int *numShipFiredLasers_;
  ShipFiredTorpedo*** shipFiredTorpedos_;
  int *numShipFiredTorpedos_;
  LaserHitShip*** laserHitShips_;
  int *numLaserHitShips_;

  // Events for the stage.
  ShipHitShip* stageShipHitShips_[MAX_SHIP_HIT_SHIPS];
  int numStageShipHitShips_;
  StageLaserHitShip* stageLaserHitShips_[MAX_LASER_HIT_SHIPS];
  int numStageLaserHitShips_;
  StageTorpedoHitShip* stageTorpedoHitShips_[MAX_TORPEDO_HIT_SHIPS];
  int numStageTorpedoHitShips_;
  ShipHitWall* stageShipHitWalls_[MAX_SHIP_HIT_WALLS];
  int numStageShipHitWalls_;
  ShipDestroyed* stageShipDestroyeds_[MAX_SHIP_DESTROYEDS];
  int numStageShipDestroyeds_;
  StageShipFiredLaser* stageShipFiredLasers_[MAX_SHIP_FIRED_LASERS];
  int numStageShipFiredLasers_;
  StageShipFiredTorpedo* stageShipFiredTorpedos_[MAX_SHIP_FIRED_TORPEDOS];
  int numStageShipFiredTorpedos_;

  public:
    SensorHandler(Team **teams, int numTeams, bool **teamVision);
    ~SensorHandler();
    virtual void handleLaserHitShip(Ship *srcShip, Ship *targetShip,
        double laserX, double laserY, double laserHeading, int time);
    virtual void handleTorpedoExploded(double x, double y, int time);
    virtual void handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
        double hitAngle, double hitForce, double hitDamage, int time);
    virtual void handleShipHitShip(Ship *hittingShip, Ship *targetShip,
        double inAngle, double inForce, double outAngle, double outForce,
        int time);
    virtual void handleShipHitWall(
        Ship *hittingShip, double bounceAngle, double bounceForce, int time);
    virtual void handleShipDestroyed(Ship *destroyedShip, int time,
        Ship **destroyerShips, int numDestroyers);
    virtual void handleShipFiredLaser(
        Ship *firingShip, double laserHeading, int time);
    virtual void handleShipFiredTorpedo(Ship *firingShip, double torpedoHeading,
        double torpedoDistance, int time);

    // Events for the ships.
    HitByShip** getHitByShips(int teamIndex);
    int numHitByShips(int teamIndex);
    void clearHitByShips(int teamIndex);
    HitByLaser** getHitByLasers(int teamIndex);
    int numHitByLasers(int teamIndex);
    void clearHitByLasers(int teamIndex);
    HitByTorpedo** getHitByTorpedos(int teamIndex);
    int numHitByTorpedos(int teamIndex);
    void clearHitByTorpedos(int teamIndex);
    ShipHitWall** getHitWalls(int teamIndex);
    int numHitWalls(int teamIndex);
    void clearHitWalls(int teamIndex);
    ShipDestroyed** getShipDestroyeds(int teamIndex);
    int numShipDestroyeds(int teamIndex);
    void clearShipDestroyeds(int teamIndex);
    ShipFiredLaser** getShipFiredLasers(int teamIndex);
    int numShipFiredLasers(int teamIndex);
    void clearShipFiredLasers(int teamIndex);
    ShipFiredTorpedo** getShipFiredTorpedos(int teamIndex);
    int numShipFiredTorpedos(int teamIndex);
    void clearShipFiredTorpedos(int teamIndex);
    LaserHitShip** getLaserHitShips(int teamIndex);
    int numLaserHitShips(int teamIndex);
    void clearLaserHitShips(int teamIndex);

    // Events for the stage.
    ShipHitShip** getStageShipHitShips();
    int numStageShipHitShips();
    void clearStageShipHitShips();
    StageLaserHitShip** getStageLaserHitShips();
    int numStageLaserHitShips();
    void clearStageLaserHitShips();
    StageTorpedoHitShip** getStageTorpedoHitShips();
    int numStageTorpedoHitShips();
    void clearStageTorpedoHitShips();
    ShipHitWall** getStageShipHitWalls();
    int numStageShipHitWalls();
    void clearStageShipHitWalls();
    ShipDestroyed** getStageShipDestroyeds();
    int numStageShipDestroyeds();
    void clearStageShipDestroyeds();
    StageShipFiredLaser** getStageShipFiredLasers();
    int numStageShipFiredLasers();
    void clearStageShipFiredLasers();
    StageShipFiredTorpedo** getStageShipFiredTorpedos();
    int numStageShipFiredTorpedos();
    void clearStageShipFiredTorpedos();
};

#endif
