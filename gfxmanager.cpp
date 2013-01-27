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

#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include "gfxmanager.h"

// TODO: make these member vars instead of globals

sf::CircleShape shipShape(SHIP_RADIUS);
sf::CircleShape shipDotShape(SHIP_DOT_RADIUS);
sf::Vector2f shipDotPoint(SHIP_DOT_POSITION, 0);
sf::CircleShape destroyedShape(SHIP_DEATH_RADIUS);
sf::RectangleShape sparkShape(
    sf::Vector2f(LASER_SPARK_LENGTH, LASER_SPARK_THICKNESS));
sf::Vector2f sparkPoint(SHIP_RADIUS, -0.5);
sf::RectangleShape laserShape(sf::Vector2f(LASER_SPEED, LASER_THICKNESS));
sf::Vector2f laserPoint(0, LASER_THICKNESS / 2);
sf::CircleShape torpedoCircleShape(TORPEDO_RADIUS, TORPEDO_RADIUS);
sf::RectangleShape torpedoRay(sf::Vector2f(TORPEDO_SIZE * 2, 1));
sf::Vector2f torpedoRayPoint(TORPEDO_SIZE, .5);
sf::CircleShape torpedoBlastShape(TORPEDO_BLAST_RADIUS, TORPEDO_BLAST_RADIUS);
sf::RectangleShape thrusterShape(sf::Vector2f(SHIP_RADIUS + THRUSTER_LENGTH,
                                              THRUSTER_THICKNESS));
sf::Vector2f thrusterPoint(0, THRUSTER_THICKNESS / 2);
sf::RectangleShape energyShape(sf::Vector2f(ENERGY_LENGTH, ENERGY_THICKNESS));
sf::RectangleShape dockEnergyShape(sf::Vector2f(DOCK_ENERGY_LENGTH,
                                                ENERGY_THICKNESS));

sf::RectangleShape **wallShapes;
int numWalls;
sf::RectangleShape **zoneShapes;
int numZones;

sf::Color* shipColors;
int* shipDotOffsets;
bool* shipDotDirections;
sf::Color* shipDeathColors;
sf::Color* laserColors;
sf::Color* thrusterColors;
sf::Color sparkColor(40, 100, 255, 255);
sf::Color torpedoColor(255, 89, 38, 255);
sf::Color blastColor(255, 128, 51, 255);
sf::Color energyColor(255, 255, 0, 255);
sf::Color zoneColor(100, 68, 68, 255);

sf::View dockView;
sf::View stageView;

sf::Font font;

int windowHeight;

GfxManager::GfxManager(bool showDock) {
  listener_ = 0;
  showDock_ = showDock;
  teams_ = 0;
  numTeams_ = 0;
  ships_ = 0;
  numShips_ = 0;
  initialized_ = false;
}

void GfxManager::setListener(GfxViewListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

void GfxManager::processMouseClick(int mouseX, int mouseY) {
  // TODO: find a nicer way to abstract this, maybe rects when building out
  //       the dock graphics
  if (listener_ != 0 && mouseX >= 0 && mouseX <= DOCK_SIZE) {
    if (mouseY >= 25 && mouseY <= 55) {
      listener_->onNewMatch();
    } else if (mouseY >= 65 && mouseY <= 105) {
      listener_->onStageClick();
    } else {
      for (int x = 0; x < numTeams_; x++) {
        if (mouseY >= 110 + (x * 30) && mouseY < 110 + ((x + 1) * 30)) {
          listener_->onTeamClick(teams_[x]->index);
        }
      }
    }
  }
}

double drawX(double x) {
  return STAGE_MARGIN + x;
}

double drawY(double y, double height) {
  return windowHeight - STAGE_MARGIN - y - height;
}

double drawY(double y) {
  return windowHeight - STAGE_MARGIN - y;
}

void GfxManager::initBbGfx(sf::RenderWindow *window, unsigned int viewHeight,
    Stage *stage, Team **teams, int numTeams, Ship **ships, int numShips,
    std::string resourcePath) {
  if (initialized_) {
    destroyBbGfx();
  }

  window->setVerticalSyncEnabled(true);
  windowHeight = viewHeight;
  teams_ = teams;
  numTeams_ = numTeams;
  ships_ = ships;
  numShips_ = numShips;

  if (!font.loadFromFile(resourcePath + FONT_NAME)) {
    exit(EXIT_FAILURE);
  }

  shipColors = new sf::Color[numShips];
  shipDeathColors = new sf::Color[numShips];
  laserColors = new sf::Color[numShips];
  thrusterColors = new sf::Color[numShips];
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    shipColors[x].r = ship->properties->shipR;
    shipColors[x].g = ship->properties->shipG;
    shipColors[x].b = ship->properties->shipB;
    shipDeathColors[x].r = ship->properties->shipR * 3 / 4;
    shipDeathColors[x].g = ship->properties->shipG * 3 / 4;
    shipDeathColors[x].b = ship->properties->shipB * 3 / 4;
    laserColors[x].r = ship->properties->laserR;
    laserColors[x].g = ship->properties->laserG;
    laserColors[x].b = ship->properties->laserB;
    thrusterColors[x].r = ship->properties->thrusterR;
    thrusterColors[x].g = ship->properties->thrusterG;
    thrusterColors[x].b = ship->properties->thrusterB;
    shipColors[x].a = thrusterColors[x].a = 255;
  }
  shipDotOffsets = new int[numShips];
  shipDotDirections = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    shipDotOffsets[x] = rand() % SHIP_DOT_FRAMES;
    shipDotDirections[x] = (rand() % 10 < 5) ? true : false;
  }
  
  shipShape.setOutlineThickness(SHIP_RADIUS * .25);
  shipShape.setFillColor(sf::Color::Black);
  shipDotShape.setOutlineThickness(0);
  destroyedShape.setOutlineThickness(2);
  destroyedShape.setFillColor(sf::Color::Transparent);

  sparkShape.setOutlineThickness(0);
  laserShape.setOutlineThickness(0);

  torpedoCircleShape.setOutlineColor(torpedoColor);
  torpedoCircleShape.setFillColor(torpedoColor);
  torpedoRay.setOutlineColor(torpedoColor);
  torpedoRay.setFillColor(torpedoColor);
  torpedoBlastShape.setOutlineColor(blastColor);
  torpedoBlastShape.setOutlineThickness(5);
  torpedoBlastShape.setFillColor(sf::Color::Transparent);

  thrusterShape.setOutlineThickness(0);
  energyShape.setOutlineColor(energyColor);
  energyShape.setFillColor(energyColor);
  dockEnergyShape.setOutlineColor(energyColor);
  dockEnergyShape.setFillColor(energyColor);
  
  numWalls = stage->getWallCount();
  wallShapes = new sf::RectangleShape*[numWalls];
  Wall **walls = stage->getWalls();
  for (int x = 0; x < numWalls; x++) {
    Wall *wall = walls[x];
    sf::RectangleShape *wallShape = new sf::RectangleShape(
        sf::Vector2f(wall->getWidth(), wall->getHeight()));
    wallShape->setPosition(drawX(wall->getLeft()),
                           drawY(wall->getBottom(), wall->getHeight()));
    wallShape->setOutlineColor(sf::Color::White);
    wallShape->setFillColor(sf::Color::White);
    wallShapes[x] = wallShape;
  }

  numZones = stage->getZoneCount();
  zoneShapes = new sf::RectangleShape*[numZones];
  Zone **zones = stage->getZones();
  for (int x = 0; x < numZones; x++) {
    Zone *zone = zones[x];
    sf::RectangleShape *zoneShape = new sf::RectangleShape(
        sf::Vector2f(zone->getWidth(), zone->getHeight()));
    zoneShape->setPosition(drawX(zone->getLeft()),
                           drawY(zone->getBottom(), zone->getHeight()));
    zoneShape->setOutlineColor(zoneColor);
    zoneShape->setFillColor(zoneColor);
    zoneShapes[x] = zoneShape;
  }

  initialized_ = true;
}

void drawWalls(sf::RenderWindow *window) {
  for (int x = 0; x < numWalls; x++) {
    window->draw(*(wallShapes[x]));
  }
}

void drawZones(sf::RenderWindow *window) {
  for (int x = 0; x < numZones; x++) {
    window->draw(*(zoneShapes[x]));
  }
}

void adjustTorpedoRayPoint(sf::RectangleShape *rayShape, double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f rayOffset = transform.transformPoint(torpedoRayPoint);
  rayShape->move(-rayOffset.x, -rayOffset.y);
}

void drawTorpedos(sf::RenderWindow *window, Stage *stage) {
  Torpedo **torpedos = stage->getTorpedos();
  int numTorpedos = stage->getTorpedoCount();
  for (int x = 0; x < numTorpedos; x++) {
    Torpedo *torpedo = torpedos[x];
    torpedoCircleShape.setPosition(drawX(torpedo->x - TORPEDO_RADIUS),
        drawY(torpedo->y - TORPEDO_RADIUS, TORPEDO_SIZE));
    window->draw(torpedoCircleShape);
    torpedoRay.setPosition(drawX(torpedo->x), drawY(torpedo->y));
    torpedoRay.setRotation(45);
    adjustTorpedoRayPoint(&torpedoRay, 45);
    window->draw(torpedoRay);
    torpedoRay.setRotation(135);
    torpedoRay.setPosition(drawX(torpedo->x), drawY(torpedo->y));
    adjustTorpedoRayPoint(&torpedoRay, 135);
    window->draw(torpedoRay);
  }
}

void drawTorpedoBlasts(sf::RenderWindow *window, int time,
                       GfxEventHandler *gfxHandler) {
  TorpedoBlastGraphic **torpedoBlasts = gfxHandler->getTorpedoBlasts();
  int numTorpedoBlasts = gfxHandler->getTorpedoBlastCount();
  
  if (numTorpedoBlasts > 0) {
    for (int x = 0; x < numTorpedoBlasts; x++) {
      TorpedoBlastGraphic *torpedoBlast = torpedoBlasts[x];
      int blastTime = time - torpedoBlast->time;
      double blastScale;
      if (blastTime < 5) {
        blastScale = ((double) (4 - blastTime)) / 4;
      } else {
        blastScale = ((double) (blastTime - 3)) / (TORPEDO_BLAST_FRAMES - 4);
      }

      double blastOffset = blastScale * TORPEDO_BLAST_RADIUS;
      torpedoBlastShape.setRadius(blastScale * TORPEDO_BLAST_RADIUS);
      torpedoBlastShape.setPosition(drawX(torpedoBlast->x - blastOffset),
          drawY(torpedoBlast->y - blastOffset, blastOffset * 2));
      window->draw(torpedoBlastShape);
    }
  }
}

void adjustThrusterPosition(sf::RectangleShape *thrusterShape, double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f thrusterOffset = transform.transformPoint(thrusterPoint);
  thrusterShape->move(-thrusterOffset.x, -thrusterOffset.y);
}

void drawThrusters(sf::RenderWindow *window, Ship **ships, int numShips) {
  for (int z = 0; z < numShips; z++) {
    Ship *ship = ships[z];
    if (ship->alive && ship->thrusterForce > 0) {
      double forceFactor = ship->thrusterForce / MAX_THRUSTER_FORCE;
      double lengthScale = THRUSTER_ZERO + (forceFactor * (1 - THRUSTER_ZERO));
      thrusterShape.setFillColor(thrusterColors[z]);
      thrusterShape.setScale(lengthScale, lengthScale);
      double rotateAngle =
          toDegrees(-normalAbsoluteAngle(ship->thrusterAngle + M_PI));
      thrusterShape.setRotation(rotateAngle);
      thrusterShape.setPosition(drawX(ship->x), drawY(ship->y));
      adjustThrusterPosition(&thrusterShape, rotateAngle);
      window->draw(thrusterShape);
    }
  }
}

// TODO: combine some with adjustThrusterPosition
void adjustLaserPosition(sf::RectangleShape *laserShape, double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f laserOffset = transform.transformPoint(laserPoint);
  laserShape->move(-laserOffset.x, -laserOffset.y);
}

void drawLasers(sf::RenderWindow *window, Stage *stage) {
  Laser **lasers = stage->getLasers();
  int numLasers = stage->getLaserCount();
  for (int x = 0; x < numLasers; x++) {
    Laser *laser = lasers[x];
    double rotateAngle = toDegrees(-normalAbsoluteAngle(laser->heading));
    laserShape.setRotation(rotateAngle);
    laserShape.setPosition(drawX(laser->x - laser->dx),
                           drawY(laser->y - laser->dy));
    laserShape.setFillColor(laserColors[laser->shipIndex]);
    adjustLaserPosition(&laserShape, rotateAngle);
    window->draw(laserShape);
  }
}

void adjustShipDotPosition(sf::CircleShape *shipDotShape, double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f dotOffset = transform.transformPoint(shipDotPoint);
  shipDotShape->move(dotOffset.x, dotOffset.y);
}

void drawShips(sf::RenderWindow *window, Ship **ships, int numShips, int time) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      shipShape.setOutlineColor(shipColors[x]);
      shipShape.setPosition(drawX(ship->x - SHIP_RADIUS),
                            drawY(ship->y - SHIP_RADIUS, SHIP_SIZE));
      window->draw(shipShape);

      shipDotShape.setFillColor(laserColors[x]);
      for (int y = 0; y < 3; y++) {
        shipDotShape.setPosition(drawX(ship->x - SHIP_DOT_RADIUS),
            drawY(ship->y - SHIP_DOT_RADIUS, SHIP_DOT_RADIUS * 2));
        double angle = (M_PI * 2 * y / 3)
            + ((((double) ((time + shipDotOffsets[x]) % SHIP_DOT_FRAMES))
                / SHIP_DOT_FRAMES) * (shipDotDirections[x] ? 2 : -2) * M_PI);
        adjustShipDotPosition(&shipDotShape, toDegrees(angle));
        window->draw(shipDotShape);
      }
    }
  }
  
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->energyEnabled) {
      energyShape.setPosition(drawX(ship->x - (ENERGY_LENGTH / 2)),
                              drawY(ship->y - SHIP_RADIUS - 8));
      energyShape.setScale(ship->energy / DEFAULT_ENERGY, 1);
      window->draw(energyShape);
    }
  }
}

void drawShipDeaths(sf::RenderWindow *window, int time,
                    GfxEventHandler *gfxHandler) {
  ShipDeathGraphic **shipDeaths = gfxHandler->getShipDeaths();
  int numShipDeaths = gfxHandler->getShipDeathCount();

  if (numShipDeaths > 0) {
    for (int x = 0; x < numShipDeaths; x++) {
      ShipDeathGraphic *shipDeath = shipDeaths[x];
      int deathTime = (time - shipDeath->time) / SHIP_DEATH_FRAME_LENGTH;
      destroyedShape.setOutlineColor(shipDeathColors[shipDeath->shipIndex]);
      for (int y = std::max(0, deathTime - 3); y < deathTime; y++) {
        double thisRadius = SHIP_DEATH_RADIUS * (1 + y);
        destroyedShape.setRadius(thisRadius);
        destroyedShape.setPosition(drawX(shipDeath->x - thisRadius),
            drawY(shipDeath->y - thisRadius, thisRadius * 2));
        window->draw(destroyedShape);
      }
    }
  }
}

void adjustLaserSparkPosition(sf::RectangleShape *sparkShape, double angle,
                              int sparkTime) {
  sf::Transform transform;
  transform.rotate(angle);
  double scale = 1 + (((double) sparkTime) / 1.25);
  transform.scale(scale, scale);
  sf::Vector2f sparkOffset = transform.transformPoint(sparkPoint);
  sparkShape->move(sparkOffset.x, sparkOffset.y);
}

void drawLaserSparks(sf::RenderWindow *window, int time,
                     GfxEventHandler *gfxHandler, Ship **ships) {
  LaserHitShipGraphic **laserHits = gfxHandler->getLaserHits();
  int numLaserHits = gfxHandler->getLaserHitCount();
  
  if (numLaserHits > 0) {
    for (int x = 0; x < numLaserHits; x++) {
      LaserHitShipGraphic *laserHit = laserHits[x];
      Ship *ship = ships[laserHit->hitShipIndex];
      int sparkTime = (time - laserHit->time);
      sparkShape.setFillColor(laserColors[laserHit->srcShipIndex]);
      for (int x = 0; x < 4; x++) {
        sparkShape.setPosition(drawX(ship->x), drawY(ship->y));
        sparkShape.setRotation(laserHit->offsets[x]);
        adjustLaserSparkPosition(&sparkShape, laserHit->offsets[x], sparkTime);
        window->draw(sparkShape);
      }
    }
  }
}

void drawNames(sf::RenderWindow *window, Ship **ships, int numShips) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->showName) {
      sf::Text text(ship->properties->name, font, 20);
      text.setColor(sf::Color::White);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(drawX(ship->x - (textRect.width / 2)),
          drawY(ship->y - SHIP_RADIUS - (ship->energyEnabled ? 10 : 4)));
      window->draw(text);
    }
  }
}

void drawStageTexts(sf::RenderWindow *window, Stage *stage, bool gameOver) {
  int numTexts = stage->getTextCount();
  if (numTexts > 0) {
    StageText **stageTexts = stage->getTexts();
    for (int x = 0; x < numTexts; x++) {
      StageText *stageText = stageTexts[x];
      sf::Text text(stageText->text, font, 28);
      text.setColor(sf::Color::White);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(drawX(stageText->x),
                       drawY(stageText->y + textRect.height));
      window->draw(text);
    }
    if (!gameOver) {
      stage->updateTextTimers();
    }
  }
}

void GfxManager::drawDock(sf::RenderWindow *window, Stage *stage) {
  // TODO: avoid a lot of extra re-initialization
  window->setView(dockView);
  sf::Text newMatchText(NEW_MATCH_TEXT, font, 18);
  newMatchText.setPosition(10, 30);
  sf::Text stageName(stage->getName(), font,  16);
  stageName.setPosition(10, 70);
  window->draw(newMatchText);
  window->draw(stageName);

  int dockTeamIndex = 0;
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    double teamEnergy = 0;
    double teamEnergyTotal = 0;
    bool showTeam = false;
    for (int x = 0; x < team->numShips; x++) {
      Ship *ship = ships_[team->firstShipIndex + x];
      showTeam = showTeam || ship->showName;
      if (ship->energyEnabled) {
        teamEnergy += ship->energy;
        teamEnergyTotal += DEFAULT_ENERGY;
      }
    }
    if (showTeam && team->shipsAlive > 0 && teamEnergyTotal > 0) {
      dockEnergyShape.setPosition(10, 130 + (dockTeamIndex * 30));
      dockEnergyShape.setScale(teamEnergy / teamEnergyTotal, 1);
      window->draw(dockEnergyShape);
    }
    if (showTeam) {
      sf::Text teamName(team->name, font, 16);
      teamName.setPosition(10, 110 + (dockTeamIndex * 30));
      window->draw(teamName);
      dockTeamIndex++;
    }
  }
}

// TODO: move all these args to class level?
void GfxManager::drawGame(sf::RenderWindow *window, Stage *stage, Ship **ships,
                          int numShips, int time, GfxEventHandler *gfxHandler,
                          bool gameOver) {
  if (showDock_) {
    drawDock(window, stage);
    window->setView(stageView);
  }
  gfxHandler->removeShipDeaths(time - SHIP_DEATH_TIME);
  gfxHandler->removeLaserHits(time - LASER_SPARK_TIME);
  gfxHandler->removeTorpedoBlasts(time - TORPEDO_BLAST_TIME);
  
  drawZones(window);
  drawTorpedos(window, stage);
  drawTorpedoBlasts(window, time, gfxHandler);
  drawWalls(window);
  drawShipDeaths(window, time, gfxHandler);
  drawLaserSparks(window, time, gfxHandler, ships);
  drawThrusters(window, ships, numShips);
  drawNames(window, ships, numShips);
  drawLasers(window, stage);
  drawShips(window, ships, numShips, time);
  drawStageTexts(window, stage, gameOver);
}

void GfxManager::updateView(sf::RenderWindow *window, unsigned int viewWidth,
                            unsigned int viewHeight) {
  unsigned int windowWidth = window->getSize().x;
  unsigned int windowHeight = window->getSize().y;
  unsigned int dockSize = (showDock_ ? DOCK_SIZE : 0);

  double widthScale = ((double) windowWidth - dockSize) / viewWidth;
  double heightScale = ((double) windowHeight) / viewHeight;
  double scale = std::min(widthScale, heightScale);
  windowWidth = floor(scale * viewWidth) + dockSize;
  windowHeight = floor(scale * viewHeight);
  window->setSize(sf::Vector2u(windowWidth, windowHeight));

  if (showDock_) {
    dockView.reset(sf::FloatRect(0, 0, dockSize, windowHeight));
    stageView.reset(sf::FloatRect(0, 0, viewWidth, viewHeight));
    float dockViewportSize = (((double) dockSize) / windowWidth);
    float stageViewportSize = 1 - dockViewportSize;
    dockView.setViewport(sf::FloatRect(0.f, 0.f, dockViewportSize, 1.f));
    stageView.setViewport(sf::FloatRect(dockViewportSize, 0.f,
                                        stageViewportSize, 1.f));
  } else {
    stageView.reset(sf::FloatRect(0, 0, viewWidth, viewHeight));
    stageView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window->setView(stageView);
  }
}

void GfxManager::destroyBbGfx() {
  if (initialized_) {
    for (int x = 0; x < numWalls; x++) {
      delete wallShapes[x];
    }
    delete wallShapes;
    for (int x = 0; x < numZones; x++) {
      delete zoneShapes[x];
    }
    delete zoneShapes;
    delete shipColors;
    delete shipDeathColors;
    delete laserColors;
    delete thrusterColors;
    delete shipDotOffsets;
    delete shipDotDirections;
    initialized_ = false;
  }
}

GfxManager::~GfxManager() {

}
