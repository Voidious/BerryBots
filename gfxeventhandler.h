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

#ifndef GFX_EVENT_HANDLER_H
#define GFX_EVENT_HANDLER_H

#include "bbutil.h"
#include "eventhandler.h"

#define MAX_SHIP_DEATHS  64

typedef struct {
  short shipIndex;
  double x;
  double y;
  int time;
} ShipDeathGraphic;

typedef struct {
  short srcShipIndex;
  short hitShipIndex;
  int time;
  short offsets[4];
} LaserHitShipGraphic;

typedef struct {
  double x;
  double y;
  int time;
} TorpedoBlastGraphic;

class GfxEventHandler : public EventHandler {
  LaserHitShipGraphic* laserHits_[MAX_LASERS];
  int numLaserHits_;
  ShipDeathGraphic* shipDeaths_[MAX_SHIP_DEATHS];
  int numShipDeaths_;
  TorpedoBlastGraphic* torpedoBlasts_[MAX_TORPEDOS];
  int numTorpedoBlasts_;

  public:
    GfxEventHandler();
    ~GfxEventHandler();
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
    virtual void handleShipDestroyed(Ship *destroyedShip, int time);
    virtual void handleShipFiredLaser(
        Ship *firingShip, double laserHeading, int time);
    virtual void handleShipFiredTorpedo(Ship *firingShip, double torpedoHeading,
        double torpedoDistance, int time);

    LaserHitShipGraphic** getLaserHits();
    int getLaserHitCount();
    void removeLaserHits(int time);
    ShipDeathGraphic** getShipDeaths();
    int getShipDeathCount();
    void removeShipDeaths(int time);
    TorpedoBlastGraphic** getTorpedoBlasts();
    int getTorpedoBlastCount();
    void removeTorpedoBlasts(int time);
};

#endif
