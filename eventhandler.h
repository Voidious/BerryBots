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

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "bbutil.h"

class EventHandler {
  public:
    virtual void handleLaserHitShip(Ship *srcShip, Ship *targetShip,
        Laser *laser, double dx, double dy, int time) = 0;
    virtual void handleTorpedoExploded(Torpedo *torpedo, int time) = 0;
    virtual void handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
        double dx, double dy, double hitAngle, double hitForce,
        double hitDamage, int time) = 0;
    // hittingShip hit targetShip, sending a momentum of inAngle / inForce,
    // receiving momentum of outAngle / outForce
    virtual void handleShipHitShip(Ship *hittingShip, Ship *targetShip,
        double inAngle, double inForce, double outAngle, double outForce,
        int time) = 0;
    virtual void handleShipHitWall(Ship *hittingShip, double bounceAngle,
        double bounceForce, int time) = 0;
    virtual void handleShipDestroyed(Ship *destroyedShip, int time,
        Ship **destroyerShips, int numDestroyers) = 0;
    virtual void handleShipFiredLaser(Ship *firingShip, Laser *laser) = 0;
    // TODO: consider combining LaserDestroyed with LaserHitShip/Wall
    virtual void handleLaserDestroyed(Laser *laser, int time) = 0;
    virtual void handleShipFiredTorpedo(Ship *firingShip, Torpedo *torpedo) = 0;
    virtual void handleTorpedoDestroyed(Torpedo *torpedo, int time) = 0;
    virtual void handleStageText(StageText *stageText) = 0;
    virtual void tooManyUserGfxRectangles(Team *team) = 0;
    virtual void tooManyUserGfxLines(Team *team) = 0;
    virtual void tooManyUserGfxCircles(Team *team) = 0;
    virtual void tooManyUserGfxTexts(Team *team) = 0;
};

#endif
