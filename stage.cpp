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

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "bbutil.h"
#include "stage.h"
#include "wall.h"
#include "zone.h"
#include "circle2d.h"
#include "point2d.h"
#include "line2d.h"

Stage::Stage(int width, int height) {
  name_ = 0;
  setSize(width, height);
  numWalls_ = 0;
  numWallLines_ = 0;
  numInnerWallLines_ = 0;
  numZones_ = 0;
  numStarts_ = 0;
  numStageShips_ = 0;
  numStageTexts_ = 0;
  startIndex_ = 0;
  for (int x = 0; x < 4; x++) {
    baseWallLines_[x] = 0;
  }
  teams_ = 0;
  numTeams_ = 0;
  ships_ = 0;
  numShips_ = 0;
  numLasers_ = 0;
  numTorpedos_ = 0;
  numEventHandlers_ = 0;
  gfxEnabled_ = false;
  numGfxRectangles_ = 0;
  numGfxLines_ = 0;
  numGfxCircles_ = 0;
  numGfxTexts_ = 0;
  userGfxDisabled_ = false;
}

void Stage::setName(char *name) {
  if (name_ != 0) {
    delete name_;
  }
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
}

char* Stage::getName() {
  return name_;
}

void Stage::setSize(int width, int height) {
  width_ = width;
  height_ = height;
}

int Stage::getWidth() {
  return width_;
}

int Stage::getHeight() {
  return height_;
}

int Stage::buildBaseWalls() {
  int i = 0;
  i += addWall(-4, -4, 4, height_ + 8, false);
  i += addWall(width_, -4, 4, height_ + 8, false);
  i += addWall(0, -4, width_, 4, false);
  i += addWall(0, height_, width_, 4, false);
  baseWallLines_[0] =
      wallLines_[numWallLines_++] = new Line2D(0, 0, width_, 0);
  baseWallLines_[1] =
      wallLines_[numWallLines_++] = new Line2D(width_, 0, width_, height_);
  baseWallLines_[2] =
      wallLines_[numWallLines_++] = new Line2D(width_, height_, 0, height_);
  baseWallLines_[3] =
      wallLines_[numWallLines_++] = new Line2D(0, height_, 0, 0);
  return i;
}

int Stage::addWall(
    int left, int bottom, int width, int height, bool addWallLines) {
  if (numWalls_ >= MAX_WALLS) {
    return 0;
  } else {
    Wall* wall = new Wall(left, bottom, width, height);
    walls_[numWalls_++] = wall;
    if (addWallLines) {
      Line2D** wallLines = wall->getLines();
      for (int x = 0; x < 4; x++) {
        innerWallLines_[numInnerWallLines_++] =
            wallLines_[numWallLines_++] = wallLines[x];
      }
    }
    return 1;
  }
}

Wall** Stage::getWalls() {
  return walls_;
}

int Stage::getWallCount() {
  return numWalls_;
}

int Stage::addZone(
    int left, int bottom, int width, int height, const char *tag) {
  if (numZones_ >= MAX_ZONES) {
    return 0;
  } else {
    if (strlen(tag) > 0) {
      zones_[numZones_++] = new Zone(left, bottom, width, height, tag);
    } else {
      zones_[numZones_++] = new Zone(left, bottom, width, height);
    }
    return 1;
  }
}

Zone** Stage::getZones() {
  return zones_;
}

int Stage::getZoneCount() {
  return numZones_;
}

bool Stage::inZone(Ship *ship, Zone *zone) {
  double shipx = ship->x;
  double shipy = ship->y;
  int zoneLeft = zone->getLeft();
  int zoneBottom = zone->getBottom();
  if (shipx >= zoneLeft && shipx <= zoneLeft + zone->getWidth()
      && shipy >= zoneBottom && shipy <= zoneBottom + zone->getHeight()) {
    return true;
  }
  return false;
}

bool Stage::inZone(Ship *ship, const char *tag) {
  for (int x = 0; x < numZones_; x++) {
    Zone *zone = zones_[x];
    if (strcmp(zone->getTag(), tag) == 0 && inZone(ship, zone)) {
      return true;
    }
  }
  return false;
}

bool Stage::inAnyZone(Ship *ship) {
  for (int x = 0; x < numZones_; x++) {
    if (inZone(ship, zones_[x])) {
      return true;
    }
  }
  return false;
}

bool Stage::touchedZone(Ship *oldShip, Ship *ship, Zone *zone) {
  if (inZone(ship, zone)) {
    return true;
  }

  Line2D *line = new Line2D(oldShip->x, oldShip->y, ship->x, ship->y);
  Line2D **zoneLines = zone->getLines();
  for (int x = 0; x < 4; x++) {
    Line2D *zoneLine = zoneLines[x];
    if (zoneLine->intersects(line)) {
      delete line;
      return true;
    }
  }
  delete line;
  return false;
}

bool Stage::touchedZone(Ship *oldShip, Ship *ship, const char *tag) {
  for (int x = 0; x < numZones_; x++) {
    Zone *zone = zones_[x];
    if (strcmp(zone->getTag(), tag) == 0 && touchedZone(oldShip, ship, zone)) {
      return true;
    }
  }
  return false;
}

bool Stage::touchedAnyZone(Ship *oldShip, Ship *ship) {
  for (int x = 0; x < numZones_; x++) {
    if (touchedZone(oldShip, ship, zones_[x])) {
      return true;
    }
  }
  return false;
}

int Stage::addStart(double x, double y) {
  if (numStarts_ >= MAX_STARTS) {
    return 0;
  } else {
    starts_[numStarts_++] = new Point2D(x, y);
    return 1;
  }
}

Point2D* Stage::getStart() {
  double x, y;
  if (startIndex_ >= numStarts_) {
    x = SHIP_RADIUS + (rand() % (width_ - SHIP_SIZE));
    y = SHIP_RADIUS + (rand() % (height_ - SHIP_SIZE));
  } else {
    Point2D *p = starts_[startIndex_++];
    x = p->getX();
    y = p->getY();
  }

  while (isShipInWall(x, y)) {
    x = limit(SHIP_RADIUS, x + (rand() % SHIP_SIZE) - SHIP_RADIUS,
        width_ - SHIP_RADIUS);
    y = limit(SHIP_RADIUS, y + (rand() % SHIP_SIZE) - SHIP_RADIUS,
        height_ - SHIP_RADIUS);
  }
  return new Point2D(x, y);
}

int Stage::getStartCount() {
  return numStarts_;
}

int Stage::addStageShip(const char *stageShipFilename) {
  if (numStageShips_ >= MAX_STAGE_SHIPS) {
    return 0;
  } else {
    char *newStageShip = new char[strlen(stageShipFilename) + 1];
    strcpy(newStageShip, stageShipFilename);
    stageShips_[numStageShips_++] = newStageShip;
    return 1;
  }
}

char** Stage::getStageShips() {
  return stageShips_;
}

int Stage::getStageShipCount() {
  return numStageShips_;
}

bool Stage::isShipInWall(double x, double y) {
  for (int z = 0; z < numWalls_; z++) {
    Wall *wall = walls_[z];
    double left = wall->getLeft();
    double bottom = wall->getBottom();
    if (x > left && x < left + wall->getWidth() && y > bottom
        && y < bottom + wall->getHeight()) {
      return true;
    }
  }

  Circle2D *shipCircle = new Circle2D(x, y, SHIP_RADIUS);
  for (int z = 0; z < numWallLines_; z++) {
    Line2D* line = wallLines_[z];
    if (shipCircle->intersects(line)) {
      delete shipCircle;
      return true;
    }
  }
  delete shipCircle;
  return false;
}

bool Stage::isShipInShip(int shipIndex, double x, double y) {
  Circle2D *shipCircle = new Circle2D(x, y, SHIP_RADIUS);
  bool shipInShip = false;
  for (int z = 0; z < numShips_ && !shipInShip; z++) {
    if (shipIndex != z && ships_[z]->alive) {
      Ship *otherShip = ships_[z];
      Circle2D *otherShipCircle =
          new Circle2D(otherShip->x, otherShip->y, SHIP_RADIUS);
      if (shipCircle->overlaps(otherShipCircle)) {
        shipInShip = true;
      }
      delete otherShipCircle;
    }
  }
  delete shipCircle;
  return shipInShip;
}

int Stage::addStageText(int gameTime, const char *text, double x, double y,
                        int fontSize, int drawTicks) {
  if (numStageTexts_ >= MAX_STAGE_TEXTS) {
    return 0;
  } else {
    char *newText = new char[strlen(text) + 1];
    strcpy(newText, text);
    StageText *stageText = new StageText;
    stageText->text = newText;
    stageText->x = x;
    stageText->y = y;
    stageText->startTime = gameTime;
    stageText->fontSize = fontSize;
    stageText->drawTicks = 1;
    stageTexts_[numStageTexts_++] = stageText;
    return 1;
  }
}

StageText** Stage::getStageTexts() {
  return stageTexts_;
}

int Stage::getStageTextCount() {
  return numStageTexts_;
}

void Stage::clearStaleStageTexts(int gameTime) {
  for (int x = 0; x < numStageTexts_; x++) {
    StageText *stageText = stageTexts_[x];
    if (gameTime - stageText->startTime >= stageText->drawTicks) {
      if (numStageTexts_ > 1) {
        stageTexts_[x] = stageTexts_[numStageTexts_ - 1];
      }
      delete stageText->text;
      delete stageText;
      numStageTexts_--;
      x--;
    }
  }
}

bool Stage::getGfxEnabled() {
  return gfxEnabled_;
}

void Stage::setGfxEnabled(bool enabled) {
  gfxEnabled_ = enabled;
}

void Stage::disableUserGfx() {
  userGfxDisabled_ = true;
}

int Stage::addUserGfxRectangle(Team *team, int gameTime, double left,
    double bottom, double width, double height, double rotation,
    RgbaColor fillColor, double outlineThickness, RgbaColor outlineColor,
    int drawTicks) {
  if (userGfxDisabled_
      || (team == 0 ? numGfxRectangles_ : team->numRectangles)
          >= MAX_USER_RECTANGLES) {
    return 0;
  } else {
    UserGfxRectangle *rectangle = new UserGfxRectangle;
    rectangle->left = left;
    rectangle->bottom = bottom;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->rotation = rotation;
    rectangle->fillR = fillColor.r;
    rectangle->fillG = fillColor.g;
    rectangle->fillB = fillColor.b;
    rectangle->fillA = fillColor.a;
    rectangle->outlineThickness = outlineThickness;
    rectangle->outlineR = outlineColor.r;
    rectangle->outlineG = outlineColor.g;
    rectangle->outlineB = outlineColor.b;
    rectangle->outlineA = outlineColor.a;
    rectangle->startTime = gameTime;
    rectangle->drawTicks = drawTicks;
    if (team == 0) {
      gfxRectangles_[numGfxRectangles_++] = rectangle;
    } else {
      team->gfxRectangles[team->numRectangles++] = rectangle;
    }
    return 1;
  }
}

UserGfxRectangle** Stage::getShipGfxRectangles(int teamIndex) {
  return teams_[teamIndex]->gfxRectangles;
}

int Stage::getShipGfxRectangleCount(int teamIndex) {
  return teams_[teamIndex]->numRectangles;
}

UserGfxRectangle** Stage::getStageGfxRectangles() {
  return gfxRectangles_;
}

int Stage::getStageGfxRectangleCount() {
  return numGfxRectangles_;
}

void Stage::clearStaleUserGfxRectangles(int gameTime) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    team->numRectangles =
        clearStaleUserGfxRectangles(gameTime, team->gfxRectangles,
                                    team->numRectangles);
  }
  numGfxRectangles_ = clearStaleUserGfxRectangles(gameTime, gfxRectangles_,
                                                  numGfxRectangles_);
}

int Stage::clearStaleUserGfxRectangles(int gameTime,
      UserGfxRectangle** gfxRectangles, int numRectangles) {
  for (int y = 0; y < numRectangles; y++) {
    UserGfxRectangle *rectangle = gfxRectangles[y];
    if (gameTime - rectangle->startTime >= rectangle->drawTicks) {
      for (int z = y; z < numRectangles - 1; z++) {
        gfxRectangles[z] = gfxRectangles[z + 1];
      }
      delete rectangle;
      numRectangles--;
      y--;
    }
  }
  return numRectangles;
}

int Stage::addUserGfxLine(Team *team, int gameTime, double x, double y,
    double angle, double length, double thickness, RgbaColor fillColor,
    double outlineThickness, RgbaColor outlineColor, int drawTicks) {
  if (userGfxDisabled_
      || (team == 0 ? numGfxLines_ : team->numLines) >= MAX_USER_LINES) {
    return 0;
  } else {
    UserGfxLine *line = new UserGfxLine;
    line->x = x;
    line->y = y;
    line->angle = angle;
    line->length = length;
    line->thickness = thickness;
    line->fillR = fillColor.r;
    line->fillG = fillColor.g;
    line->fillB = fillColor.b;
    line->fillA = fillColor.a;
    line->outlineThickness = outlineThickness;
    line->outlineR = outlineColor.r;
    line->outlineG = outlineColor.g;
    line->outlineB = outlineColor.b;
    line->outlineA = outlineColor.a;
    line->startTime = gameTime;
    line->drawTicks = drawTicks;
    if (team == 0) {
      gfxLines_[numGfxLines_++] = line;
    } else {
      team->gfxLines[team->numLines++] = line;
    }
    return 1;
  }
}

UserGfxLine** Stage::getShipGfxLines(int teamIndex) {
  return teams_[teamIndex]->gfxLines;
}

int Stage::getShipGfxLineCount(int teamIndex) {
  return teams_[teamIndex]->numLines;
}

UserGfxLine** Stage::getStageGfxLines() {
  return gfxLines_;
}

int Stage::getStageGfxLineCount() {
  return numGfxLines_;
}

void Stage::clearStaleUserGfxLines(int gameTime) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    team->numLines = clearStaleUserGfxLines(gameTime, team->gfxLines,
                                            team->numLines);
  }
  numGfxLines_ = clearStaleUserGfxLines(gameTime, gfxLines_, numGfxLines_);
}

int Stage::clearStaleUserGfxLines(int gameTime, UserGfxLine** gfxLines,
                                  int numLines) {
  for (int y = 0; y < numLines; y++) {
    UserGfxLine *line = gfxLines[y];
    if (gameTime - line->startTime >= line->drawTicks) {
      for (int z = y; z < numLines - 1; z++) {
        gfxLines[z] = gfxLines[z + 1];
      }
      delete line;
      numLines--;
      y--;
    }
  }
  return numLines;
}

int Stage::addUserGfxCircle(Team *team, int gameTime, double x, double y,
    double radius, RgbaColor fillColor, double outlineThickness,
    RgbaColor outlineColor, int drawTicks) {
  if (userGfxDisabled_
      || (team == 0 ? numGfxCircles_ : team->numCircles) >= MAX_USER_CIRCLES) {
    return 0;
  } else {
    UserGfxCircle *circle = new UserGfxCircle;
    circle->x = x;
    circle->y = y;
    circle->radius = radius;
    circle->fillR = fillColor.r;
    circle->fillG = fillColor.g;
    circle->fillB = fillColor.b;
    circle->fillA = fillColor.a;
    circle->outlineThickness = outlineThickness;
    circle->outlineR = outlineColor.r;
    circle->outlineG = outlineColor.g;
    circle->outlineB = outlineColor.b;
    circle->outlineA = outlineColor.a;
    circle->startTime = gameTime;
    circle->drawTicks = drawTicks;
    if (team == 0) {
      gfxCircles_[numGfxCircles_++] = circle;
    } else {
      team->gfxCircles[team->numCircles++] = circle;
    }
    return 1;
  }
}

UserGfxCircle** Stage::getShipGfxCircles(int teamIndex) {
  return teams_[teamIndex]->gfxCircles;
}

int Stage::getShipGfxCircleCount(int teamIndex) {
  return teams_[teamIndex]->numCircles;
}

UserGfxCircle** Stage::getStageGfxCircles() {
  return gfxCircles_;
}

int Stage::getStageGfxCircleCount() {
  return numGfxCircles_;
}

void Stage::clearStaleUserGfxCircles(int gameTime) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    team->numCircles = clearStaleUserGfxCircles(gameTime, team->gfxCircles,
                                                team->numCircles);
  }
  numGfxCircles_ = clearStaleUserGfxCircles(gameTime, gfxCircles_,
                                            numGfxCircles_);
}

int Stage::clearStaleUserGfxCircles(int gameTime, UserGfxCircle** gfxCircles,
                                    int numCircles) {
  for (int y = 0; y < numCircles; y++) {
    UserGfxCircle *circle = gfxCircles[y];
    if (gameTime - circle->startTime >= circle->drawTicks) {
      for (int z = y; z < numCircles - 1; z++) {
        gfxCircles[z] = gfxCircles[z + 1];
      }
      delete circle;
      numCircles--;
      y--;
    }
  }
  return numCircles;
}

int Stage::addUserGfxText(Team *team, int gameTime, const char *text,
    double x, double y, int fontSize, RgbaColor textColor, int drawTicks) {
  if (userGfxDisabled_
      || (team == 0 ? numGfxTexts_ : team->numTexts) >= MAX_USER_TEXTS) {
    return 0;
  } else {
    UserGfxText *userText = new UserGfxText;
    char *newText = new char[strlen(text) + 1];
    strcpy(newText, text);
    userText->text = newText;
    userText->x = x;
    userText->y = y;
    userText->fontSize = fontSize;
    userText->textR = textColor.r;
    userText->textG = textColor.g;
    userText->textB = textColor.b;
    userText->textA = textColor.a;
    userText->startTime = gameTime;
    userText->drawTicks = drawTicks;
    if (team == 0) {
      gfxTexts_[numGfxTexts_++] = userText;
    } else {
      team->gfxTexts[team->numTexts++] = userText;
    }
    return 1;
  }
}

UserGfxText** Stage::getShipGfxTexts(int teamIndex) {
  return teams_[teamIndex]->gfxTexts;
}

int Stage::getShipGfxTextCount(int teamIndex) {
  return teams_[teamIndex]->numTexts;
}

UserGfxText** Stage::getStageGfxTexts() {
  return gfxTexts_;
}

int Stage::getStageGfxTextCount() {
  return numGfxTexts_;
}

void Stage::clearStaleUserGfxTexts(int gameTime) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    team->numTexts = clearStaleUserGfxTexts(gameTime, team->gfxTexts,
                                            team->numTexts);
  }
  numGfxTexts_ = clearStaleUserGfxTexts(gameTime, gfxTexts_, numGfxTexts_);
}

int Stage::clearStaleUserGfxTexts(int gameTime, UserGfxText** gfxTexts,
                                  int numTexts) {
  for (int y = 0; y < numTexts; y++) {
    UserGfxText *userText = gfxTexts[y];
    if (gameTime - userText->startTime >= userText->drawTicks) {
      for (int z = y; z < numTexts - 1; z++) {
        gfxTexts[z] = gfxTexts[z + 1];
      }
      delete userText;
      numTexts--;
      y--;
    }
  }
  return numTexts;
}

// TODO: It's very confusing that this class takes ownership of ships.
void Stage::setTeamsAndShips(
    Team **teams, int numTeams, Ship **ships, int numShips) {
  teams_ = teams;
  numTeams_ = numTeams;
  ships_ = ships;
  numShips_ = numShips;
}

void Stage::moveAndCheckCollisions(
    Ship **oldShips, Ship **ships, int numShips, int gameTime) {
  ShipMoveData *shipData = new ShipMoveData[numShips];

  // Calculate initial movement and decide on a common sub-tick interval.
  int intervals = 1;
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    ShipMoveData shipDatum = shipData[x];
    shipDatum.initialized = false;
    shipDatum.shipCircle = 0;
    shipDatum.nextShipCircle = 0;
    if (ship->alive) {
      shipDatum.initialized = true;
      Ship *oldShip = oldShips[x];
  
      double force = ship->thrusterForce;
      double forceAngle = ship->thrusterAngle;
      shipDatum.dxSpeed = cos(forceAngle) * force;
      shipDatum.dySpeed = sin(forceAngle) * force;
      shipDatum.xSpeed = (cos(ship->heading) * ship->speed) + shipDatum.dxSpeed;
      shipDatum.ySpeed = (sin(ship->heading) * ship->speed) + shipDatum.dySpeed;
      ship->x += shipDatum.xSpeed;
      ship->y += shipDatum.ySpeed;
      ship->hitWall = false;
      ship->hitShip = false;
      setSpeedAndHeading(oldShip, ship, &shipDatum);
  
      shipDatum.shipCircle = new Circle2D(oldShip->x, oldShip->y, SHIP_RADIUS);
      shipDatum.nextShipCircle = new Circle2D(0, 0, SHIP_RADIUS);
      shipDatum.wallCollision = false;
      shipDatum.minWallImpactDiff = M_PI;
      shipDatum.wallImpactAngle = 0;
      shipDatum.wallImpactLine = 0;
      shipDatum.shipCollision = false;
      shipDatum.shipCollisionData = new ShipCollisionData*[numShips];
      for (int y = 0; y < numShips; y++) {
        shipDatum.shipCollisionData[y] = 0;
      }
      shipDatum.stopped = false;
  
      double moveDistance =
          sqrt(square(ship->x - oldShip->x) + square(ship->y - oldShip->y));
      int moveIntervals = ceil(moveDistance / COLLISION_FRAME);
      intervals = std::max(intervals, moveIntervals);
    }
    shipData[x] = shipDatum;
  }

  for (int x = 0; x < numShips; x++) {
    if (ships[x]->alive) {
      shipData[x].dx = shipData[x].xSpeed / intervals;
      shipData[x].dy = shipData[x].ySpeed / intervals;
    }
  }

  for (int x = 0; x < intervals; x++) {
    // Move ships one interval and check for wall collisions.
    for (int y = 0; y < numShips; y++) {
      if (ships[y]->alive) {
        ShipMoveData *shipDatum = &(shipData[y]);
        if (!shipDatum->stopped) {
          Ship *oldShip = oldShips[y];
          shipDatum->nextShipCircle->setPosition(
              oldShip->x + (shipDatum->dx * (x + 1)),
              oldShip->y + (shipDatum->dy * (x + 1)));
          for (int z = 0; z < numWallLines_; z++) {
            Line2D* line = wallLines_[z];
            Point2D *p1 = 0;
            Point2D *p2 = 0;
            if (shipDatum->nextShipCircle->intersects(line, &p1, &p2)) {
              double thisX = shipDatum->shipCircle->h();
              double thisY = shipDatum->shipCircle->k();
              bool valid = true;
              if (p1 != 0) {
                Line2D vertexSightLine1(thisX, thisY,
                    p1->getX() - (signum(p1->getX() - thisX) * VERTEX_FUDGE),
                    p1->getY() - (signum(p1->getY() - thisY) * VERTEX_FUDGE));
                valid = hasVision(&vertexSightLine1);
                delete p1;
              }
              if (p2 != 0) {
                if (valid) {
                  Line2D vertexSightLine2(thisX, thisY,
                      p2->getX() - (signum(p2->getX() - thisX) * VERTEX_FUDGE),
                      p2->getY() - (signum(p2->getY() - thisY) * VERTEX_FUDGE));
                  valid = hasVision(&vertexSightLine2);
                }
                delete p2;
              }
              if (valid) {
                shipDatum->stopped = shipDatum->wallCollision = true;
                double heading = ships[y]->heading;
                double angle1 = line->theta() - M_PI_2;
                double angleDiff1 = abs(normalRelativeAngle(heading - angle1));
                double angle2 = line->theta() + M_PI_2;
                double angleDiff2 = abs(normalRelativeAngle(heading - angle2));
                if (angleDiff1 < shipDatum->minWallImpactDiff) {
                  shipDatum->minWallImpactDiff = angleDiff1;
                  shipDatum->wallImpactAngle = angle1;
                  shipDatum->wallImpactLine = line;
                }
                if (angleDiff2 < shipDatum->minWallImpactDiff) {
                  shipDatum->minWallImpactDiff = angleDiff2;
                  shipDatum->wallImpactAngle = angle2;
                  shipDatum->wallImpactLine = line;
                }
              }
            }
          }
        }
      }
    }

    // Check for ship-ship collisions.
    for (int y = 0; y < numShips; y++) {
      ShipMoveData *shipDatum = &(shipData[y]);
      if (ships[y]->alive && !shipDatum->stopped) {
        for (int z = 0; z < numShips; z++) {
          if (y != z && ships[z]->alive) {
            ShipMoveData *shipDatum2 = &(shipData[z]);
            if (shipDatum->nextShipCircle->overlaps(shipDatum2->shipCircle)
                || (!shipDatum2->stopped
                    && shipDatum->nextShipCircle->overlaps(
                        shipDatum2->nextShipCircle))) {
              shipDatum->stopped = shipDatum2->stopped =
                  shipDatum->shipCollision = shipDatum2->shipCollision = true;
              if (shipDatum->shipCollisionData[z] == 0) {
                shipDatum->shipCollisionData[z] = new ShipCollisionData;
              }
              if (shipDatum2->shipCollisionData[y] == 0) {
                shipDatum2->shipCollisionData[y] = new ShipCollisionData;
              }
            }
          }
        }
      }
    }

    // Update for next tick and clear old locations.
    for (int y = 0; y < numShips; y++) {
      if (ships[y]->alive) {
        ShipMoveData *shipDatum = &(shipData[y]);
        if (!shipDatum->stopped) {
          shipDatum->shipCircle->setPosition(
              shipDatum->nextShipCircle->h(), shipDatum->nextShipCircle->k());
        }
      }
    }
  }

  // Calculate impact of wall collisions.
  for (int x = 0; x < numShips; x++) {
    if (ships[x]->alive) {
      ShipMoveData *shipDatum = &(shipData[x]);
      if (shipDatum->wallCollision) {
        Ship *ship = ships[x];
        Ship *oldShip = oldShips[x];
        ship->hitWall = true;
        ship->x = shipDatum->shipCircle->h();
        ship->y = shipDatum->shipCircle->k();
        if (shipStopped(oldShip, ship)) {
          shipDatum->xSpeed = shipDatum->dxSpeed;
          shipDatum->ySpeed = shipDatum->dySpeed;
          setSpeedAndHeading(oldShip, ship, shipDatum);
        }
        double bounceForce = abs(ship->speed * cos(shipDatum->minWallImpactDiff)
            * (1 + WALL_BOUNCE));
        double bounceAngle = shipDatum->wallImpactAngle + M_PI;
        shipDatum->xSpeed += cos(bounceAngle) * bounceForce;
        shipDatum->ySpeed += sin(bounceAngle) * bounceForce;
        setSpeedAndHeading(oldShip, ship, shipDatum);
        for (int z = 0; z < numEventHandlers_; z++) {
          eventHandlers_[z]->handleShipHitWall(
              ship, bounceAngle, bounceForce, gameTime);
        }
      }
    }
  }

  // Update x/y/heading/speeds for ships with ship-ship collisions.
  for (int x = 0; x < numShips; x++) {
    ShipMoveData *shipDatum = &(shipData[x]);
    if (ships[x]->alive && shipDatum->shipCollision) {
      Ship *ship = ships[x];
      ship->hitShip = true;
      ship->x = shipDatum->shipCircle->h();
      ship->y = shipDatum->shipCircle->k();
      if (shipStopped(oldShips[x], ship)) {
        shipDatum->xSpeed = shipDatum->dxSpeed;
        shipDatum->ySpeed = shipDatum->dySpeed;
        setSpeedAndHeading(oldShips[x], ship, shipDatum);
      }
    }
  }

  // Calculate momentum to be transferred between all colliding ships.
  for (int x = 0; x < numShips; x++) {
    ShipMoveData *shipDatum = &(shipData[x]);
    Ship *ship = ships[x];
    if (ship-> alive && shipDatum->shipCollision) {
      double totalForce = 0;
      for (int y = 0; y < numShips; y++) {
        if (x != y && ships[y]->alive) {
          ShipCollisionData *collisionData = shipDatum->shipCollisionData[y];
          if (collisionData != 0) {
            collisionData->angle =
                atan2(ships[y]->y - ship->y, ships[y]->x - ship->x);
            collisionData->force =
                cos(ship->heading - collisionData->angle) * ship->speed;
            totalForce += collisionData->force;
          }
        }
      }
      if (totalForce > ship->speed) {
        for (int y = 0; y < numShips; y++) {
          if (x != y) {
            ShipCollisionData *collisionData = shipDatum->shipCollisionData[y];
            if (collisionData != 0) {
              collisionData->force *= ship->speed / totalForce;
            }
          }
        }
      }
    }
  }

  // Apply momentum transfers.
  for (int x = 0; x < numShips; x++) {
    ShipMoveData *shipDatum = &(shipData[x]);
    if (ships[x]->alive && shipDatum->shipCollision) {
      for (int y = 0; y < numShips; y++) {
        if (x != y && ships[y]->alive) {
          ShipMoveData *shipDatum2 = &(shipData[y]);
          ShipCollisionData *collisionData = shipDatum->shipCollisionData[y];
          if (collisionData != 0) {
            double xForce = cos(collisionData->angle) * collisionData->force;
            double yForce = sin(collisionData->angle) * collisionData->force;
            shipDatum->xSpeed -= xForce;
            shipDatum->ySpeed -= yForce;
            setSpeedAndHeading(oldShips[x], ships[x], shipDatum);
            shipDatum2->xSpeed += xForce;
            shipDatum2->ySpeed += yForce;
            setSpeedAndHeading(oldShips[y], ships[y], shipDatum2);

            ShipCollisionData *collisionData2 =
                shipDatum2->shipCollisionData[x];
            for (int z = 0; z < numEventHandlers_; z++) {
              eventHandlers_[z]->handleShipHitShip(ships[x], ships[y],
                  collisionData->angle, collisionData->force,
                  collisionData2->angle, collisionData2->force, gameTime);
            }
          }
        }
      }
    }
  }

  // Check for laser-ship collisions, laser-wall collisions, log destroys and
  // damage, remove dead lasers.
  bool **laserHits = new bool*[numShips];
  for (int x = 0; x < numShips; x++) {
    laserHits[x] = new bool[numShips];
    for (int y = 0; y < numShips; y++) {
      laserHits[x][y] = false;
    }
  }
  bool *wasAlive = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    wasAlive[x] = ships[x]->alive;
  }

  // For lasers fired this tick, check if they intersect any other ships at
  // their initial position (0-25 from origin) before moving the first time.
  checkLaserShipCollisions(ships, shipData, numShips, laserHits, numLasers_,
                           gameTime, true);

  // Move lasers one whole tick.
  for (int x = 0; x < numLasers_; x++) {
    Laser *laser = lasers_[x];
    laser->x += laser->dx;
    laser->y += laser->dy;
    laserLines_[x]->shift(laser->dx, laser->dy);
  }
  
  checkLaserShipCollisions(ships, shipData, numShips, laserHits, numLasers_,
                           gameTime, false);

  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (wasAlive[x] && !ship->alive) {
      int numDestroyers = 0;
      for (int y = 0; y < numShips; y++) {
        if (laserHits[y][x]) {
          numDestroyers++;
        }
      }
      Ship **destroyers = new Ship*[numDestroyers];
      int destroyerIndex = 0;
      for (int y = 0; y < numShips; y++) {
        if (laserHits[y][x]) {
          destroyers[destroyerIndex++] = ships[y];
        }
      }
      
      for (int y = 0; y < numShips; y++) {
        if (laserHits[y][x]) {
          double destroyScore = 1.0 / numDestroyers;
          if (ship->teamIndex == ships[y]->teamIndex) {
            ships[y]->friendlyKills += destroyScore;
          } else {
            ships[y]->kills += destroyScore;
          }
        }
      }

      for (int z = 0; z < numEventHandlers_; z++) {
        eventHandlers_[z]->handleShipDestroyed(ship, gameTime, destroyers,
                                               numDestroyers);
      }
      delete destroyers;
    }
  }
  for (int x = 0; x < numLasers_; x++) {
    Laser *laser = lasers_[x];
    Line2D *laserLine = laserLines_[x];
    for (int y = 0; y < numWallLines_ && !lasers_[x]->dead; y++) {
      Line2D *wallLine = wallLines_[y];
      if (wallLine->intersects(laserLine)) {
        lasers_[x]->dead = true;
      }
    }
    if (laser->dead) {
      if (numLasers_ > 1) {
        lasers_[x] = lasers_[numLasers_ - 1];
        laserLines_[x] = laserLines_[numLasers_ - 1];
      }
      delete laser;
      delete laserLine;
      numLasers_--;
      x--;
    }
  }
  for (int x = 0; x < numShips; x++) {
    delete laserHits[x];
  }
  delete laserHits;

  // Move torpedoes and check for collisions.
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    wasAlive[x] = ship->alive;
  }
  bool **torpedoHits = new bool*[numShips];
  for (int x = 0; x < numShips; x++) {
    torpedoHits[x] = new bool[numShips];
    for (int y = 0; y < numShips; y++) {
      torpedoHits[x][y] = false;
    }
  }
  for (int x = 0; x < numTorpedos_; x++) {
    Torpedo *torpedo = torpedos_[x];
    double distanceRemaining = torpedo->distance - torpedo->distanceTraveled;
    if (distanceRemaining >= TORPEDO_SPEED) {
      torpedo->x += torpedo->dx;
      torpedo->y += torpedo->dy;
      torpedo->distanceTraveled += TORPEDO_SPEED;
    } else {
      double blastX = torpedo->x + cos(torpedo->heading) * distanceRemaining;
      double blastY = torpedo->y + sin(torpedo->heading) * distanceRemaining;
      for (int y = 0; y < numShips; y++) {
        Ship *ship = ships[y];
        if (ship->alive) {
          double distSq = square(blastX - ship->x) + square(blastY - ship->y);
          if (distSq < square(TORPEDO_BLAST_RADIUS)) {
            int firingShipIndex = torpedo->shipIndex;
            torpedoHits[firingShipIndex][y] = true;
            ShipMoveData *shipDatum = &(shipData[y]);
            double blastDistance = sqrt(distSq);
            double blastFactor =
                square(1.0 - (blastDistance / TORPEDO_BLAST_RADIUS));
            double blastForce = blastFactor * TORPEDO_BLAST_FORCE;
            double blastDamage = blastFactor
                * (ship->energyEnabled ? TORPEDO_BLAST_DAMAGE : 0);
            double blastAngle = atan2(ship->y - blastY, ship->x - blastX);
            double damageScore = (blastDamage / DEFAULT_ENERGY);
            if (ship->teamIndex == ships[firingShipIndex]->teamIndex) {
              ships[firingShipIndex]->friendlyDamage += damageScore;
            } else {
              ships[firingShipIndex]->damage += damageScore;
            }
            ship->energy -= blastDamage;
            shipDatum->xSpeed += cos(blastAngle) * blastForce;
            shipDatum->ySpeed += sin(blastAngle) * blastForce;
            setSpeedAndHeading(oldShips[y], ship, shipDatum);

            for (int z = 0; z < numEventHandlers_; z++) {
              eventHandlers_[z]->handleTorpedoHitShip(ships[torpedo->shipIndex],
                  ship, blastAngle, blastForce, blastDamage, gameTime);
            }
  
            if (ship->energy <= 0) {
              ship->alive = false;
            }
          }
        }
      }

      for (int z = 0; z < numEventHandlers_; z++) {
        eventHandlers_[z]->handleTorpedoExploded(blastX, blastY, gameTime);
      }

      if (numTorpedos_ > 1) {
        torpedos_[x] = torpedos_[numTorpedos_ - 1];
      }
      delete torpedo;
      numTorpedos_--;
      x--;
    }
  }
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (wasAlive[x] && !ship->alive) {
      int numDestroyers = 0;
      for (int y = 0; y < numShips; y++) {
        if (torpedoHits[y][x]) {
          numDestroyers++;
        }
      }
      Ship **destroyers = new Ship*[numDestroyers];
      int destroyerIndex = 0;
      for (int y = 0; y < numShips; y++) {
        if (torpedoHits[y][x]) {
          destroyers[destroyerIndex++] = ships[y];
        }
      }

      for (int y = 0; y < numShips; y++) {
        if (torpedoHits[y][x]) {
          double destroyScore = 1.0 / numDestroyers;
          if (ship->teamIndex == ships[y]->teamIndex) {
            ships[y]->friendlyKills += destroyScore;
          } else {
            ships[y]->kills += destroyScore;
          }
        }
      }
      for (int z = 0; z < numEventHandlers_; z++) {
        eventHandlers_[z]->handleShipDestroyed(ship, gameTime, destroyers,
                                               numDestroyers);
      }
      delete destroyers;
    }
  }

  for (int x = 0; x < numShips; x++) {
    delete torpedoHits[x];
  }
  delete torpedoHits;
  delete wasAlive;

  for (int x = 0; x < numShips; x++) {
    ShipMoveData *shipDatum = &(shipData[x]);
    if (shipDatum->initialized) {
      delete shipDatum->shipCircle;
      delete shipDatum->nextShipCircle;
      for (int y = 0; y < numShips; y++) {
        ShipCollisionData *collisionData = shipDatum->shipCollisionData[y];
        if (collisionData != 0) {
          delete collisionData;
        }
      }
      delete shipDatum->shipCollisionData;
    }
  }
  delete shipData;
}

void Stage::checkLaserShipCollisions(Ship **ships, ShipMoveData *shipData,
    int numShips, bool **laserHits, int numLasers, int gameTime,
    bool firstTickLasers) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      for (int y = 0; y < numLasers; y++) {
        Laser *laser = lasers_[y];
        if (((firstTickLasers && laser->fireTime == gameTime
                 && laser->shipIndex != ship->index)
            || !firstTickLasers)
            && shipData[x].shipCircle->intersects(laserLines_[y])) {
          int firingShipIndex = laser->shipIndex;
          laserHits[firingShipIndex][x] = true;
          double laserDamage = (ship->energyEnabled ? LASER_DAMAGE : 0);
          double damageScore = (laserDamage / DEFAULT_ENERGY);
          if (ship->teamIndex == ships[firingShipIndex]->teamIndex) {
            ships[firingShipIndex]->friendlyDamage += damageScore;
          } else {
            ships[firingShipIndex]->damage += damageScore;
          }
          ship->energy -= laserDamage;
          for (int z = 0; z < numEventHandlers_; z++) {
            eventHandlers_[z]->handleLaserHitShip(ships[laser->shipIndex],
                ship, laser->x, laser->y, laser->heading, gameTime);
          }

          if (ship->energy <= 0) {
            ship->alive = false;
          }
          laser->dead = true;
        }
      }
    }
  }
}

void Stage::setSpeedAndHeading(
    Ship *oldShip, Ship *ship, ShipMoveData *shipDatum) {
  // Try to round off rounding errors so bot authors can check speed == 0
  ship->speed =
      round(sqrt(square(shipDatum->xSpeed) + square(shipDatum->ySpeed)), 5);
  ship->heading = ((abs(ship->speed) < 0.000001)
      ? oldShip->heading : atan2(shipDatum->ySpeed, shipDatum->xSpeed));
}

bool Stage::shipStopped(Ship *oldShip, Ship *ship) {
  return (abs(oldShip->x - ship->x) < 0.00001
      && abs(oldShip->y - ship->y) < 0.00001
      && (oldShip->hitWall || oldShip->hitShip));
}

void Stage::updateTeamVision(
    Team **teams, int numTeams, Ship** ships, int numShips, bool** teamVision) {
  for (int x = 0; x < numTeams; x++) {
    Team *team = teams[x];
    for (int y = 0; y < numShips; y++) {
      Ship *ship = ships[y];
      teamVision[x][y] =
          (x != ship->teamIndex) && (team->shipsAlive > 0) && (ship->alive);
    }
  }

  if (numInnerWallLines_ == 0) {
    return;
  }

  for (int x = 0; x < numTeams; x++) {
    Team *team = teams[x];
    for (int y = 0; y < numShips; y++) {
      Ship *visibleShip = ships[y];
      if (x != visibleShip->teamIndex && visibleShip->alive) {
        bool teamHasVision = false;
        for (int z = 0; z < team->numShips && !teamHasVision; z++) {
          int teamShipIndex = team->firstShipIndex + z;
          Ship *teamShip = ships[teamShipIndex];
          if (teamShip->alive) {
            Line2D visionLine(
                teamShip->x, teamShip->y, visibleShip->x, visibleShip->y);
            teamHasVision = hasVision(&visionLine);
          }
        }
        teamVision[x][y] = teamHasVision;
      }
    }
  }
}

bool Stage::hasVision(Line2D *visionLine) {
  for (int z = 0; z < numInnerWallLines_; z++) {
    Line2D* wallLine = innerWallLines_[z];
    if (wallLine->intersects(visionLine)) {
      return false;
    }
  }
  return true;
}

void Stage::updateShipPosition(Ship *ship, double x, double y) {
  while (isShipInWall(x, y) || isShipInShip(ship->index, x, y)) {
    x = limit(SHIP_RADIUS, x + (rand() % SHIP_SIZE) - SHIP_RADIUS,
              width_ - SHIP_RADIUS);
    y = limit(SHIP_RADIUS, y + (rand() % SHIP_SIZE) - SHIP_RADIUS,
              height_ - SHIP_RADIUS);
  }
  ship->x = x;
  ship->y = y;
}

int Stage::fireLaser(Ship *ship, double heading, int gameTime) {
  if (ship->laserGunHeat > 0 || numLasers_ >= MAX_LASERS) {
    return 0;
  } else {
    for (int z = 0; z < numEventHandlers_; z++) {
      eventHandlers_[z]->handleShipFiredLaser(ship, heading, gameTime);
    }

    double cosHeading = cos(heading);
    double sinHeading = sin(heading);
    double laserX = ship->x + (cosHeading * LASER_SPEED);
    double laserY = ship->y + (sinHeading * LASER_SPEED);
    Line2D laserStartLine(ship->x, ship->y, laserX, laserY);
    if (hasVision(&laserStartLine)) {
      Laser *laser = new Laser;
      laser->shipIndex = ship->index;
      laser->fireTime = gameTime;
      double dx = cosHeading * LASER_SPEED;
      double dy = sinHeading * LASER_SPEED;
      laser->x = laserX;
      laser->y = laserY;
      laser->heading = heading;
      laser->dx = dx;
      laser->dy = dy;
      laser->dead = false;
      lasers_[numLasers_] = laser;
      laserLines_[numLasers_++] = new Line2D(
          laser->x - laser->dx, laser->y - laser->dy, laser->x, laser->y);
    }

    return 1;
  }
}

int Stage::fireTorpedo(
    Ship *ship, double heading, double distance, int gameTime) {
  if (ship->torpedoGunHeat > 0 || numTorpedos_ >= MAX_TORPEDOS) {
    return 0;
  } else {
    Torpedo *torpedo = new Torpedo;
    torpedo->shipIndex = ship->index;
    double cosHeading = cos(heading);
    double sinHeading = sin(heading);
    double dx = cosHeading * TORPEDO_SPEED;
    double dy = sinHeading * TORPEDO_SPEED;
    torpedo->x = ship->x + (cosHeading * SHIP_RADIUS);
    torpedo->y = ship->y + (sinHeading * SHIP_RADIUS);
    torpedo->heading = heading;
    torpedo->distance = distance;
    torpedo->dx = dx;
    torpedo->dy = dy;
    torpedo->distanceTraveled = SHIP_RADIUS;
    torpedos_[numTorpedos_++] = torpedo;

    for (int z = 0; z < numEventHandlers_; z++) {
      eventHandlers_[z]->handleShipFiredTorpedo(
          ship, heading, distance, gameTime);
    }

    return 1;
  }
}

Laser** Stage::getLasers() {
  return lasers_;
}

int Stage::getLaserCount() {
  return numLasers_;
}

Torpedo** Stage::getTorpedos() {
  return torpedos_;
}

int Stage::getTorpedoCount() {
  return numTorpedos_;
}

void Stage::destroyShip(Ship *ship, int gameTime) {
  if (ship->alive) {
    ship->alive = false;
    for (int z = 0; z < numEventHandlers_; z++) {
      eventHandlers_[z]->handleShipDestroyed(ship, gameTime, 0, 0);
    }
  }
}

int Stage::addEventHandler(EventHandler *eventHandler) {
  if (numEventHandlers_ >= MAX_EVENT_HANDLERS) {
    return 0;
  } else {
    eventHandlers_[numEventHandlers_++] = eventHandler;
    return 1;
  }
}

void Stage::reset() {
  startIndex_ = 0;
  for (int x = 0; x < numLasers_; x++) {
    delete lasers_[x];
    delete laserLines_[x];
  }
  numLasers_ = 0;
  for (int x = 0; x < numTorpedos_; x++) {
    delete torpedos_[x];
  }
  numTorpedos_ = 0;
  for (int x = 0; x < numStageTexts_; x++) {
    delete stageTexts_[x]->text;
    delete stageTexts_[x];
  }
  numStageTexts_ = 0;
}

Stage::~Stage() {
  if (name_ != 0) {
    delete name_;
  }
  for (int x = 0; x < numWalls_; x++) {
    delete walls_[x];
  }
  for (int x = 0; x < numZones_; x++) {
    delete zones_[x];
  }
  for (int x = 0; x < numStarts_; x++) {
    delete starts_[x];
  }
  for (int x = 0; x < 4; x++) {
    if (baseWallLines_[x] != 0) {
      delete baseWallLines_[x];
    }
  }
  for (int x = 0; x < numStageTexts_; x++) {
    delete stageTexts_[x]->text;
    delete stageTexts_[x];
  }
  for (int x = 0; x < numLasers_; x++) {
    delete lasers_[x];
    delete laserLines_[x];
  }
  for (int x = 0; x < numTorpedos_; x++) {
    delete torpedos_[x];
  }
  for (int x = 0; x < numStageShips_; x++) {
    delete stageShips_[x];
  }
  delete ships_;
}
