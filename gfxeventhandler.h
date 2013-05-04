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

#ifndef GFX_EVENT_HANDLER_H
#define GFX_EVENT_HANDLER_H

#include "bbutil.h"
#include "eventhandler.h"

#define MAX_SHIP_DEATHS     256
#define NUM_LASER_SPARKS    4
#define MAX_TORPEDO_SPARKS  30

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
  double x;
  double y;
  double dx;
  double dy;
  short offsets[NUM_LASER_SPARKS];
} LaserHitShipGraphic;

typedef struct {
  double x;
  double y;
  int time;
} TorpedoBlastGraphic;

typedef struct {
  short srcShipIndex;
  short hitShipIndex;
  int time;
  double x;
  double y;
  double dx;
  double dy;
  short numTorpedoSparks;
  short offsets[MAX_TORPEDO_SPARKS];
  short speeds[MAX_TORPEDO_SPARKS];
} TorpedoHitShipGraphic;

class GfxEventHandler : public EventHandler {
  LaserHitShipGraphic* laserHits_[MAX_LASERS];
  int numLaserHits_;
  TorpedoHitShipGraphic* torpedoHits_[MAX_TORPEDOS];
  int numTorpedoHits_;
  ShipDeathGraphic* shipDeaths_[MAX_SHIP_DEATHS];
  int numShipDeaths_;
  TorpedoBlastGraphic* torpedoBlasts_[MAX_TORPEDOS];
  int numTorpedoBlasts_;

  public:
    GfxEventHandler();
    ~GfxEventHandler();
    virtual void handleLaserHitShip(Ship *srcShip, Ship *targetShip, double dx,
        double dy, double laserX, double laserY, double laserHeading, int time);
    virtual void handleTorpedoExploded(double x, double y, int time);
    virtual void handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
        double dx, double dy, double hitAngle, double hitForce,
        double hitDamage, int time);
    virtual void handleShipHitShip(Ship *hittingShip, Ship *targetShip,
        double inAngle, double inForce, double outAngle, double outForce,
        int time) {};
    virtual void handleShipHitWall(
        Ship *hittingShip, double bounceAngle, double bounceForce, int time) {};
    virtual void handleShipDestroyed(Ship *destroyedShip, int time,
        Ship **destroyerShips, int numDestroyers);
    virtual void handleShipFiredLaser(Ship *firingShip, Laser *laser) {};
    virtual void handleShipFiredTorpedo(Ship *firingShip, Torpedo *torpedo) {};
    virtual void tooManyUserGfxRectangles(Team *team) {};
    virtual void tooManyUserGfxLines(Team *team) {};
    virtual void tooManyUserGfxCircles(Team *team) {};
    virtual void tooManyUserGfxTexts(Team *team) {};

    LaserHitShipGraphic** getLaserHits();
    int getLaserHitCount();
    void removeLaserHits(int time);
    TorpedoHitShipGraphic** getTorpedoHits();
    int getTorpedoHitCount();
    void removeTorpedoHits(int time);
    ShipDeathGraphic** getShipDeaths();
    int getShipDeathCount();
    void removeShipDeaths(int time);
    TorpedoBlastGraphic** getTorpedoBlasts();
    int getTorpedoBlastCount();
    void removeTorpedoBlasts(int time);
};

#endif
