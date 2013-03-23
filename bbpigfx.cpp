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
#include <stdlib.h>
#include <algorithm>
#include "VG/vgu.h"
#include "shapes.h"
#include "bbconst.h"
#include "bbutil.h"
#include "stage.h"
#include "rectangle.h"
#include "wall.h"
#include "zone.h"
#include "bbpigfx.h"
#include "gfxeventhandler.h"

float BLACK[4] = {0, 0, 0, 1};
float WHITE[4] = {1, 1, 1, 1};
float GRAY[4] = {.7, .7, .7, 1};
float ZONE_COLOR[4] = {.3, .2, .2, 1};
float TORPEDO_COLOR[4] = {1, .35, .15, 1};
float BLAST_COLOR[4] = {1, .5, .2, 1};
float ENERGY_COLOR[4] = {1, 1, 0, 1};
float DESTROYED_COLOR[4] = {0, 0, 1, 1};

VGPath bgPath;
VGPath shipPath;
VGPath shipDotPath;
VGPath thrusterPath;
VGPath laserPath;
VGPath torpedoPath;
VGPath shipDeathPaths[SHIP_DEATH_FRAMES];
VGPath laserSparkPaths[LASER_SPARK_FRAMES];
VGPath torpedoBlastPaths[TORPEDO_BLAST_FRAMES];
VGPath energyPath;
VGPath** zonePaths;
int numZonePaths;
VGPath** wallPaths;
int numWallPaths;
int* shipDotOffsets;
bool* shipDotDirections;

VGPaint* shipPaints;
VGPaint* shipDeathPaints;
VGPaint* laserPaints;
VGPaint* thrusterPaints;
int numShipPaints;
VGPaint blackPaint;
VGPaint whitePaint;
VGPaint grayPaint;
VGPaint zonePaint;
VGPaint torpedoPaint;
VGPaint blastPaint;
VGPaint sparkPaint;
VGPaint energyPaint;
VGPaint destroyedPaint;

int stageLeft;
int stageBottom;
double scale;

void initVgGfx(int screenWidth, int screenHeight, Stage *stage, Ship **ships,
    int numShips) {
  int desiredWidth = stage->getWidth() + 50;
  int desiredHeight = stage->getHeight() + 50;
  scale = std::min(1.0, std::min(((double) screenWidth) / desiredWidth,
                                 ((double) screenHeight) / desiredHeight));
  stageLeft = ((screenWidth / scale) / 2) - (stage->getWidth() / 2);
  stageBottom = ((screenHeight / scale) / 2) - (stage->getHeight() / 2);

  bgPath = newpath();
  vguRect(bgPath, 0, 0, screenWidth, screenHeight);
  shipPath = newpath();
  vguEllipse(shipPath, 0, 0, SHIP_SIZE, SHIP_SIZE);
  shipDotPath = newpath();
  vguEllipse(shipDotPath, 0, 0, SHIP_DOT_SIZE, SHIP_DOT_SIZE);
  thrusterPath = newpath();
  vguLine(thrusterPath, 0, 0, DRAW_SHIP_RADIUS + THRUSTER_LENGTH, 0);
  laserPath = newpath();
  vguLine(laserPath, 0, 0, LASER_SPEED, 0);
  torpedoPath = newpath();
  vguEllipse(torpedoPath, 0, 0, TORPEDO_SIZE, TORPEDO_SIZE);
  vguLine(torpedoPath, -TORPEDO_RADIUS - 3, -TORPEDO_RADIUS - 3,
      TORPEDO_RADIUS + 3, TORPEDO_RADIUS + 3);
  vguLine(torpedoPath, -TORPEDO_RADIUS - 3, TORPEDO_RADIUS + 3,
      TORPEDO_RADIUS + 3, -TORPEDO_RADIUS - 3);
  energyPath = newpath();
  vguLine(energyPath, 0, 0, ENERGY_LENGTH, 0);

  shipPaints = new VGPaint[numShips];
  shipDeathPaints = new VGPaint[numShips];
  laserPaints = new VGPaint[numShips];
  thrusterPaints = new VGPaint[numShips];
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    VGfloat color[4];
    RGBA(ship->properties->shipR, ship->properties->shipG,
         ship->properties->shipB, 1, color);
    shipPaints[x] = createPaint(color);
    VGfloat color2[4];
    RGBA(ship->properties->shipR * 3 / 4, ship->properties->shipG * 3 / 4,
         ship->properties->shipB * 3 / 4, 1, color2);
    shipDeathPaints[x] = createPaint(color2);
    VGfloat color3[4];
    RGBA(ship->properties->laserR, ship->properties->laserG,
         ship->properties->laserB, 1, color3);
    laserPaints[x] = createPaint(color3);
    VGfloat color4[4];
    RGBA(ship->properties->thrusterR, ship->properties->thrusterG,
         ship->properties->thrusterB, 1, color4);
    thrusterPaints[x] = createPaint(color4);
  }
  numShipPaints = numShips;

  shipDotOffsets = new int[numShips];
  shipDotDirections = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    shipDotOffsets[x] = rand() % SHIP_DOT_FRAMES;
    shipDotDirections[x] = (rand() % 10 < 5) ? true : false;
  }

  for (int x = 0; x < SHIP_DEATH_FRAMES; x++) {
    shipDeathPaths[x] = newpath();
    vguEllipse(shipDeathPaths[x], 0, 0, (x + 1) * 12, (x + 1) * 12);
  }
  for (int x = 0; x < LASER_SPARK_FRAMES; x++) {
    laserSparkPaths[x] = newpath();
    vguLine(laserSparkPaths[x], DRAW_SHIP_RADIUS + (x * LASER_SPARK_LENGTH), 0,
        DRAW_SHIP_RADIUS + ((x + 1) * LASER_SPARK_LENGTH), 0);
  }
  for (int x = 0; x < TORPEDO_BLAST_FRAMES; x++) {
    torpedoBlastPaths[x] = newpath();
    int blastSize;
    if (x < 5) {
      blastSize = ceil(TORPEDO_BLAST_RADIUS * 2.0 * (4 - x) / 4);
    } else {
      blastSize = ceil(
          TORPEDO_BLAST_RADIUS * 2.0 * (x - 3) / (TORPEDO_BLAST_FRAMES - 4));
    }
    vguEllipse(torpedoBlastPaths[x], 0, 0, blastSize, blastSize);
  }

  Zone **zones = stage->getZones();
  numZonePaths = stage->getZoneCount();
  zonePaths = new VGPath*[numZonePaths];
  for (int x = 0; x < numZonePaths; x++) {
    Rectangle *zone = zones[x];
    VGPath path = newpath();
    vguRect(path, stageLeft + zone->getLeft(), stageBottom + zone->getBottom(),
        zone->getWidth(), zone->getHeight());
    zonePaths[x] = new VGPath;
    *(zonePaths[x]) = path;
  }

  Wall **walls = stage->getWalls();
  numWallPaths = stage->getWallCount();
  wallPaths = new VGPath*[numWallPaths];
  for (int x = 0; x < numWallPaths; x++) {
    Rectangle *wall = walls[x];
    VGPath path = newpath();
    vguRect(path, stageLeft + wall->getLeft(), stageBottom + wall->getBottom(),
        wall->getWidth(), wall->getHeight());
    wallPaths[x] = new VGPath;
    *(wallPaths[x]) = path;
  }

  blackPaint = createPaint(BLACK);
  whitePaint = createPaint(WHITE);
  grayPaint = createPaint(GRAY);
  zonePaint = createPaint(ZONE_COLOR);
  torpedoPaint = createPaint(TORPEDO_COLOR);
  blastPaint = createPaint(BLAST_COLOR);
  energyPaint = createPaint(ENERGY_COLOR);
  destroyedPaint = createPaint(DESTROYED_COLOR);
}

VGPaint createPaint(float *color) {
  VGPaint paint = vgCreatePaint();
  vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
  vgSetParameterfv(paint, VG_PAINT_COLOR, 4, color);
  return paint;
}

void destroyVgGfx() {
  vgDestroyPath(bgPath);
  vgDestroyPath(shipPath);
  vgDestroyPath(shipDotPath);
  vgDestroyPath(thrusterPath);
  vgDestroyPath(laserPath);
  vgDestroyPath(torpedoPath);
  vgDestroyPath(energyPath);

  for (int x = 0; x < numZonePaths; x++) {
    vgDestroyPath(*(zonePaths[x]));
    delete zonePaths[x];
  }
  delete zonePaths;
  for (int x = 0; x < numWallPaths; x++) {
    vgDestroyPath(*(wallPaths[x]));
    delete wallPaths[x];
  }
  delete wallPaths;

  for (int x = 0; x < SHIP_DEATH_FRAMES; x++) {
    vgDestroyPath(shipDeathPaths[x]);
  }
  for (int x = 0; x < LASER_SPARK_FRAMES; x++) {
    vgDestroyPath(laserSparkPaths[x]);
  }
  for (int x = 0; x < TORPEDO_BLAST_FRAMES; x++) {
    vgDestroyPath(torpedoBlastPaths[x]);
  }

  for (int x = 0; x < numShipPaints; x++) {
    vgDestroyPaint(shipPaints[x]);
    vgDestroyPaint(shipDeathPaints[x]);
    vgDestroyPaint(laserPaints[x]);
    vgDestroyPaint(thrusterPaints[x]);
  }
  delete shipPaints;
  delete shipDeathPaints;
  delete laserPaints;
  delete thrusterPaints;

  vgDestroyPaint(blackPaint);
  vgDestroyPaint(whitePaint);
  vgDestroyPaint(grayPaint);
  vgDestroyPaint(zonePaint);
  vgDestroyPaint(torpedoPaint);
  vgDestroyPaint(blastPaint);
  vgDestroyPaint(energyPaint);
  vgDestroyPaint(destroyedPaint);

  delete shipDotOffsets;
  delete shipDotDirections;
}

void drawZones() {
  vgLoadIdentity();
  vgScale(scale, scale);
  if (numZonePaths > 0) {
    StrokeWidth(1);
    vgSetPaint(zonePaint, VG_FILL_PATH | VG_STROKE_PATH);
  }
  for (int x = 0; x < numZonePaths; x++) {
    vgDrawPath(*(zonePaths[x]), VG_FILL_PATH | VG_STROKE_PATH);
  }
}

void drawWalls() {
  vgLoadIdentity();
  vgScale(scale, scale);
  if (numWallPaths > 0) {
    StrokeWidth(1);
    vgSetPaint(whitePaint, VG_FILL_PATH | VG_STROKE_PATH);
  }
  for (int x = 0; x < numWallPaths; x++) {
    vgDrawPath(*(wallPaths[x]), VG_FILL_PATH | VG_STROKE_PATH);
  }
}

void drawShips(Ship **ships, int numShips, int time) {
  StrokeWidth(SHIP_OUTLINE_THICKNESS);
  vgSetPaint(blackPaint, VG_FILL_PATH);
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      vgLoadIdentity();
      vgScale(scale, scale);
      vgSetPaint(shipPaints[x], VG_STROKE_PATH);
      vgTranslate(stageLeft + ship->x, stageBottom + ship->y);
      vgDrawPath(shipPath, VG_FILL_PATH | VG_STROKE_PATH);
    }
  }

  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      vgSetPaint(laserPaints[x], VG_FILL_PATH);
      double angle = (((double) ((time + shipDotOffsets[x]) % SHIP_DOT_FRAMES))
          / SHIP_DOT_FRAMES) * (shipDotDirections[x] ? 2 : -2) * M_PI;
      for (int x = 0; x < 3; x++) {
        angle += 2 * M_PI / 3;
        vgLoadIdentity();
        vgScale(scale, scale);
        vgTranslate(stageLeft + ship->x, stageBottom + ship->y);
        vgRotate(toDegrees(angle));
        vgTranslate(SHIP_DOT_POSITION, 0);
        vgDrawPath(shipDotPath, VG_FILL_PATH);
      }
    }
  }

  StrokeWidth(ENERGY_THICKNESS);
  vgSetPaint(energyPaint, VG_STROKE_PATH | VG_FILL_PATH);
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->energyEnabled) {
      vgLoadIdentity();
      vgScale(scale, scale);
      vgTranslate(stageLeft + ship->x - (ENERGY_LENGTH / 2),
                  stageBottom + ship->y - DRAW_SHIP_RADIUS - 8);
      VGfloat lengthScale = (ship->energy / DEFAULT_ENERGY);
      vgScale(lengthScale, 1);
      vgDrawPath(energyPath, VG_STROKE_PATH);
    }
  }
}

void drawShipDeaths(int time, GfxEventHandler *gfxHandler) {
  ShipDeathGraphic **shipDeaths = gfxHandler->getShipDeaths();
  int numShipDeaths = gfxHandler->getShipDeathCount();

  if (numShipDeaths > 0) {
    StrokeWidth(2);
    for (int x = 0; x < numShipDeaths; x++) {
      ShipDeathGraphic *shipDeath = shipDeaths[x];
      vgSetPaint(shipDeathPaints[shipDeath->shipIndex], VG_STROKE_PATH);
      vgLoadIdentity();
      vgScale(scale, scale);
      vgTranslate(stageLeft + shipDeath->x, stageBottom + shipDeath->y);
      int deathTime = (time - shipDeath->time) / SHIP_DEATH_FRAME_LENGTH;
      for (int y = std::max(0, deathTime - 3); y < deathTime; y++) {
        vgDrawPath(shipDeathPaths[y], VG_STROKE_PATH);
      }
    }
  }
}

void drawLaserSparks(int time, GfxEventHandler *gfxHandler, Ship **ships) {
  LaserHitShipGraphic **laserHits = gfxHandler->getLaserHits();
  int numLaserHits = gfxHandler->getLaserHitCount();

  if (numLaserHits > 0) {
    StrokeWidth(LASER_SPARK_THICKNESS);
    for (int x = 0; x < numLaserHits; x++) {
      LaserHitShipGraphic *laserHit = laserHits[x];
      Ship *ship = ships[laserHit->hitShipIndex];
      int sparkTime = (time - laserHit->time);
      vgSetPaint(laserPaints[laserHit->srcShipIndex], VG_STROKE_PATH);
      for (int y = 0; y < 4; y++) {
        vgLoadIdentity();
        vgScale(scale, scale);
        vgTranslate(stageLeft + ship->x, stageBottom + ship->y);
        vgRotate(laserHit->offsets[y]);
        vgDrawPath(laserSparkPaths[sparkTime], VG_STROKE_PATH);
      }
    }
  }
}

void drawTorpedoBlasts(int time, GfxEventHandler *gfxHandler) {
  TorpedoBlastGraphic **torpedoBlasts = gfxHandler->getTorpedoBlasts();
  int numTorpedoBlasts = gfxHandler->getTorpedoBlastCount();

  if (numTorpedoBlasts > 0) {
    StrokeWidth(5);
    vgSetPaint(blastPaint, VG_STROKE_PATH);
    for (int x = 0; x < numTorpedoBlasts; x++) {
      TorpedoBlastGraphic *torpedoBlast = torpedoBlasts[x];
      vgLoadIdentity();
      vgScale(scale, scale);
      vgTranslate(stageLeft + torpedoBlast->x, stageBottom + torpedoBlast->y);
      vgDrawPath(torpedoBlastPaths[time - torpedoBlast->time], VG_STROKE_PATH);
    }
  }
}

void drawThrusters(Ship **ships, int numShips) {
  StrokeWidth(THRUSTER_THICKNESS);
  for (int z = 0; z < numShips; z++) {
    Ship *ship = ships[z];
    if (ship->alive && ship->thrusterForce > 0) {
      vgSetPaint(thrusterPaints[z], VG_STROKE_PATH);
      vgLoadIdentity();
      vgScale(scale, scale);
      vgTranslate(stageLeft + ship->x, stageBottom + ship->y);
      double forceFactor = ship->thrusterForce / MAX_THRUSTER_FORCE;
      VGfloat lengthScale =
          THRUSTER_ZERO + (forceFactor * (1 - THRUSTER_ZERO));
      vgScale(lengthScale, lengthScale);
      vgRotate(toDegrees(normalAbsoluteAngle(ship->thrusterAngle + M_PI)));
      vgDrawPath(thrusterPath, VG_STROKE_PATH);
    }
  }
}

void drawLasers(Stage *stage) {
  Laser **lasers = stage->getLasers();
  int numLasers = stage->getLaserCount();
  if (numLasers > 0) {
    StrokeWidth(LASER_THICKNESS);
  }
  for (int x = 0; x < numLasers; x++) {
    Laser *laser = lasers[x];
    vgSetPaint(laserPaints[laser->shipIndex], VG_STROKE_PATH);
    vgLoadIdentity();
    vgScale(scale, scale);
    vgTranslate(stageLeft + laser->x - laser->dx,
        stageBottom + laser->y - laser->dy);
    vgRotate(toDegrees(normalAbsoluteAngle(laser->heading)));
    vgDrawPath(laserPath, VG_STROKE_PATH);
  }
}

void drawTorpedos(Stage *stage) {
  Torpedo **torpedos = stage->getTorpedos();
  int numTorpedos = stage->getTorpedoCount();
  if (numTorpedos > 0) {
    StrokeWidth(1);
    vgSetPaint(torpedoPaint, VG_STROKE_PATH | VG_FILL_PATH);
  }
  for (int x = 0; x < numTorpedos; x++) {
    Torpedo *torpedo = torpedos[x];
    vgLoadIdentity();
    vgScale(scale, scale);
    vgTranslate(stageLeft + torpedo->x, stageBottom + torpedo->y);
    vgDrawPath(torpedoPath, VG_STROKE_PATH | VG_FILL_PATH);
  }
}

void drawNames(Ship **ships, int numShips) {
  vgLoadIdentity();
  vgScale(scale, scale);
  StrokeWidth(.01);
  vgSetPaint(grayPaint, VG_STROKE_PATH | VG_FILL_PATH);
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->showName) {
      SansTextMid(stageLeft + ship->x, stageBottom + ship->y - DRAW_SHIP_RADIUS - 30,
                  ship->properties->name, 12);
    }
  }
}

void drawStageTexts(Stage *stage, int time) {
  stage->clearStaleStageTexts(time);
  int numTexts = stage->getStageTextCount();
  if (numTexts > 0) {
    vgLoadIdentity();
    vgScale(scale, scale);
    StrokeWidth(.01);
    vgSetPaint(whitePaint, VG_STROKE_PATH | VG_FILL_PATH);
    StageText **stageTexts = stage->getStageTexts();
    for (int x = 0; x < numTexts; x++) {
      StageText *stageText = stageTexts[x];
      SansText(stageLeft + stageText->x, stageBottom + stageText->y,
          stageText->text, limit(MIN_STAGE_TEXT_FONT_SIZE, (stageText->fontSize * 2 / 3),
                                 MAX_STAGE_TEXT_FONT_SIZE));
    }
  }
}

void drawGame(int screenWidth, int screenHeight, Stage *stage, Ship **ships,
    int numShips, int time, GfxEventHandler *gfxHandler) {

  gfxHandler->removeShipDeaths(time - SHIP_DEATH_TIME);
  gfxHandler->removeLaserHits(time - LASER_SPARK_TIME);
  gfxHandler->removeTorpedoBlasts(time - TORPEDO_BLAST_TIME);

  Start(screenWidth, screenHeight);

  vgSetPaint(blackPaint, VG_FILL_PATH | VG_STROKE_PATH);
  vgDrawPath(bgPath, VG_FILL_PATH | VG_STROKE_PATH);

  drawZones();
  drawTorpedos(stage);
  drawTorpedoBlasts(time, gfxHandler);
  drawWalls();
  drawShipDeaths(time, gfxHandler);
  drawLaserSparks(time, gfxHandler, ships);
  drawThrusters(ships, numShips);
  drawNames(ships, numShips);
  drawLasers(stage);
  drawShips(ships, numShips, time);
  drawStageTexts(stage, time);

  End();
}
