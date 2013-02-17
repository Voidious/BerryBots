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

#ifndef STAGE_H
#define STAGE_H

#include "bbconst.h"
#include "bbutil.h"
#include "circle2d.h"
#include "line2d.h"
#include "point2d.h"
#include "wall.h"
#include "zone.h"
#include "eventhandler.h"

// Check if we have vision to intersection points with walls to ensure that
// we're not hitting the far side of a wall. Don't test all the way to
// intersection point or it will intersect with wall itself.
#define VERTEX_FUDGE        0.0001
#define MAX_EVENT_HANDLERS  4

typedef struct {
  double angle;
  double force;
} ShipCollisionData;

typedef struct {
  double xSpeed;
  double ySpeed;
  double dxSpeed;
  double dySpeed;
  double dx;
  double dy;
  Circle2D *shipCircle;
  Circle2D *nextShipCircle;
  bool wallCollision;
  double minWallImpactDiff;
  double wallImpactAngle;
  Line2D *wallImpactLine;
  bool shipCollision;
  ShipCollisionData** shipCollisionData;
  bool stopped;
} ShipMoveData;

typedef struct {
  int x;
  int y;
  char *text;
  int startTime;
  int drawTicks;
} StageText;

class Stage {
  char *name_;
  int width_, height_;
  int numWalls_, numWallLines_, numInnerWallLines_;
  int numZones_, numStarts_, numStageShips_, numTexts_, startIndex_;
  Wall* walls_[MAX_WALLS];
  Line2D* wallLines_[MAX_WALLS * 4];
  Line2D* innerWallLines_[MAX_WALLS * 4];
  Line2D* baseWallLines_[4];
  Zone* zones_[MAX_ZONES];
  Point2D* starts_[MAX_STARTS];
  char* stageShips_[MAX_STAGE_SHIPS];
  StageText* stageTexts_[MAX_STAGE_TEXTS];
  Team** teams_;
  int numTeams_;
  Ship** ships_;
  int numShips_;
  Laser* lasers_[MAX_LASERS];
  Line2D* laserLines_[MAX_LASERS];
  int numLasers_;
  Torpedo* torpedos_[MAX_TORPEDOS];
  int numTorpedos_;
  EventHandler* eventHandlers_[MAX_EVENT_HANDLERS];
  int numEventHandlers_;

  public:
    Stage(int width, int height);
    ~Stage();
    void setName(char *name);
    char* getName();
    void setSize(int width, int height);
    int getWidth();
    int getHeight();

    int buildBaseWalls();
    int addWall(int left, int bottom, int width, int height, bool addWallLines);
    Wall** getWalls();
    int getWallCount();
    Line2D** getWallLines();
    int getWallLinesCount();

    int addZone(int left, int bottom, int width, int height, const char *tag);
    Zone** getZones();
    int getZoneCount();
    bool inZone(Ship *ship, const char* tag);
    bool inAnyZone(Ship *ship);
    bool touchedZone(Ship *oldShip, Ship *ship, const char* tag);
    bool touchedAnyZone(Ship *oldShip, Ship *ship);

    int addStart(double x, double y);
    Point2D* getStart();
    int getStartCount();

    int addStageShip(const char *stageShipFilename);
    char** getStageShips();
    int getStageShipCount();

    int addText(int x, int y, const char *text, int gameTime, int drawTicks);
    StageText** getTexts();
    int getTextCount();
    void clearStaleTexts(int gameTime);

    void setTeamsAndShips(
        Team **teams, int numTeams, Ship **ships, int numShips);
    void moveAndCheckCollisions(
        Ship **oldShips, Ship **ships, int numShips, int gameTime);
    void updateTeamVision(Team **teams, int numTeams, Ship **ships,
        int numShips, bool **teamVision);
    void updateShipPosition(Ship *ship, double x, double y);
    int fireLaser(Ship *ship, double heading, int gameTime);
    int fireTorpedo(Ship *ship, double heading, double distance, int gameTime);
    Laser** getLasers();
    int getLaserCount();
    Torpedo** getTorpedos();
    int getTorpedoCount();
    void destroyShip(Ship *ship, int gameTime);
    int addEventHandler(EventHandler *eventHandler);
    void reset();
  private:
    void checkLaserShipCollisions(Ship **ships, ShipMoveData *shipData,
        int numShips, bool *wasAlive, bool **laserHits, int numLasers,
        int gameTime, bool firstTickLasers);
    bool isShipInWall(double x, double y);
    bool isShipInShip(int shipIndex, double x, double y);
    void setSpeedAndHeading(Ship *oldShip, Ship *ship, ShipMoveData *shipData);
    bool shipStopped(Ship *ship1, Ship *ship2);
    bool hasVision(Line2D *visionLine);
    bool inZone(Ship *ship, Zone *zone);
    bool touchedZone(Ship *oldShip, Ship *ship, Zone *zone);
};

#endif
