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
#include <string.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include "stage.h"
#include "gfxmanager.h"

// TODO: make these member vars instead of globals

sf::CircleShape shipShape(DRAW_SHIP_RADIUS);
sf::CircleShape shipDotShape(SHIP_DOT_RADIUS);
sf::Vector2f shipDotPoint(SHIP_DOT_POSITION, 0);
sf::CircleShape destroyedShape(SHIP_DEATH_RADIUS);
sf::RectangleShape sparkShape(
    sf::Vector2f(LASER_SPARK_LENGTH, LASER_SPARK_THICKNESS));
sf::Vector2f sparkPoint(DRAW_SHIP_RADIUS, -0.5);
sf::RectangleShape laserShape(sf::Vector2f(LASER_SPEED, LASER_THICKNESS));
sf::Vector2f laserPoint(0, LASER_THICKNESS / 2);
sf::CircleShape torpedoCircleShape(TORPEDO_RADIUS, TORPEDO_RADIUS);
sf::RectangleShape torpedoRay(sf::Vector2f(TORPEDO_SIZE * 2, 1));
sf::Vector2f torpedoRayPoint(TORPEDO_SIZE, .5);
sf::CircleShape torpedoBlastShape(TORPEDO_BLAST_RADIUS, TORPEDO_BLAST_RADIUS);
sf::RectangleShape thrusterShape(sf::Vector2f(DRAW_SHIP_RADIUS + THRUSTER_LENGTH,
                                              THRUSTER_THICKNESS));
sf::Vector2f thrusterPoint(0, THRUSTER_THICKNESS / 2);
sf::RectangleShape energyShape(sf::Vector2f(ENERGY_LENGTH, ENERGY_THICKNESS));
sf::RectangleShape dockEnergyShape(sf::Vector2f(DOCK_ENERGY_LENGTH,
                                                ENERGY_THICKNESS));
sf::RectangleShape dockLineShape(sf::Vector2f(1, 4096));
sf::RectangleShape dockMarginShape(sf::Vector2f(8, 4096));

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
sf::Color dockLineColor(100, 100, 100);

sf::View dockView;
sf::View stageView;

sf::Font font;

int windowHeight;

GfxManager::GfxManager(bool showDock) {
  showDock_ = showDock;
  newMatchButton_ = 0;
  packageShipButton_ = 0;
  packageStageButton_ = 0;
  stageButton_ = 0;
  teamButtons_ = 0;
  pauseButton_ = 0;
  playButton_ = 0;
  restartButton_ = 0;
  listener_ = 0;
  teams_ = 0;
  numTeams_ = 0;
  ships_ = 0;
  numShips_ = 0;
  stage_ = 0;
  initialized_ = false;
}

GfxManager::~GfxManager() {

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
  stage_ = stage;
  
  if (!font.loadFromFile(resourcePath + FONT_NAME)) {
    exit(EXIT_FAILURE);
  }

  initDockItems(window);
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
  
  shipShape.setOutlineThickness(SHIP_OUTLINE_THICKNESS);
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
  dockLineShape.setOutlineThickness(0);
  dockLineShape.setFillColor(dockLineColor);
  dockLineShape.setPosition(DOCK_SIZE - 1, 0);
  dockMarginShape.setOutlineThickness(0);
  dockMarginShape.setFillColor(sf::Color::Black);
  dockMarginShape.setPosition(DOCK_SIZE - 9, 0);
  
  numWalls = stage->getWallCount();
  wallShapes = new sf::RectangleShape*[numWalls];
  Wall **walls = stage->getWalls();
  for (int x = 0; x < numWalls; x++) {
    Wall *wall = walls[x];
    sf::RectangleShape *wallShape = new sf::RectangleShape(                                                          sf::Vector2f(wall->getWidth(), wall->getHeight()));
    wallShape->setPosition(adjustX(wall->getLeft()),
                           adjustY(wall->getBottom(), wall->getHeight()));
    wallShape->setOutlineColor(sf::Color::White);
    wallShape->setFillColor(sf::Color::White);
    wallShapes[x] = wallShape;
  }
  
  numZones = stage->getZoneCount();
  zoneShapes = new sf::RectangleShape*[numZones];
  Zone **zones = stage->getZones();
  for (int x = 0; x < numZones; x++) {
    Zone *zone = zones[x];
    sf::RectangleShape *zoneShape = new sf::RectangleShape(                                                           sf::Vector2f(zone->getWidth(), zone->getHeight()));
    zoneShape->setPosition(adjustX(zone->getLeft()),
                           adjustY(zone->getBottom(), zone->getHeight()));
    zoneShape->setOutlineColor(zoneColor);
    zoneShape->setFillColor(zoneColor);
    zoneShapes[x] = zoneShape;
  }
  
  initialized_ = true;
}

void GfxManager::initDockItems(sf::RenderWindow *window) {
  sf::Shape** newShapes = new sf::Shape*[2];
  newShapes[0] = new sf::RectangleShape(sf::Vector2f(18, 22));
  newShapes[0]->move(-12, -10);
  newShapes[0]->setOutlineThickness(2);
  newShapes[0]->setFillColor(sf::Color::Black);
  newShapes[1] = new sf::RectangleShape(sf::Vector2f(18, 22));
  newShapes[1]->move(-6, -16);
  newShapes[1]->setOutlineThickness(2);
  newShapes[1]->setFillColor(sf::Color::Black);
  newMatchButton_ = new DockItem(newShapes, 2, 10, 10, 40, 40, "New Match",
      &font, DOCK_BUTTON_FONT_SIZE, 26, 50, "N", DOCK_SHORTCUT_FONT_SIZE);

  sf::Shape** packageShipShapes = new sf::Shape*[4];
  packageShipShapes[0] = new sf::CircleShape(SHIP_RADIUS);
  packageShipShapes[0]->move(-SHIP_RADIUS, -SHIP_RADIUS);
  packageShipShapes[0]->setOutlineThickness(2);
  packageShipShapes[0]->setFillColor(sf::Color::Black);
  packageShipShapes[1] = new sf::CircleShape(SHIP_DOT_RADIUS);
  packageShipShapes[1]->move(-SHIP_DOT_RADIUS, -SHIP_DOT_RADIUS);
  adjustShipDotPosition((sf::CircleShape *) packageShipShapes[1], 15);
  packageShipShapes[2] = new sf::CircleShape(SHIP_DOT_RADIUS);
  packageShipShapes[2]->move(-SHIP_DOT_RADIUS, -SHIP_DOT_RADIUS);
  adjustShipDotPosition((sf::CircleShape *) packageShipShapes[2], 135);
  packageShipShapes[3] = new sf::CircleShape(SHIP_DOT_RADIUS);
  packageShipShapes[3]->move(-SHIP_DOT_RADIUS, -SHIP_DOT_RADIUS);
  adjustShipDotPosition((sf::CircleShape *) packageShipShapes[3], 255);
  packageShipButton_ = new DockItem(packageShipShapes, 4, 59, 10, 40, 40,
      "Package Ship", &font, DOCK_BUTTON_FONT_SIZE, 17, 50, "H",
      DOCK_SHORTCUT_FONT_SIZE);

  sf::Shape** packageStageShapes = new sf::Shape*[5];
  packageStageShapes[0] = new sf::RectangleShape(sf::Vector2f(26, 18));
  packageStageShapes[0]->move(-13, -9);
  packageStageShapes[0]->setOutlineThickness(2);
  packageStageShapes[0]->setFillColor(sf::Color::Black);
  packageStageShapes[1] = new sf::RectangleShape(sf::Vector2f(2, 2));
  packageStageShapes[1]->move(-7, 3);
  packageStageShapes[2] = new sf::RectangleShape(sf::Vector2f(2, 2));
  packageStageShapes[2]->move(-7, -5);
  packageStageShapes[3] = new sf::RectangleShape(sf::Vector2f(2, 2));
  packageStageShapes[3]->move(5, -5);
  packageStageShapes[4] = new sf::RectangleShape(sf::Vector2f(2, 2));
  packageStageShapes[4]->move(5, 3);
  packageStageButton_ = new DockItem(packageStageShapes, 5, 108, 10, 40, 40,
      "Package Stage", &font, DOCK_BUTTON_FONT_SIZE, 9, 50, "T",
      DOCK_SHORTCUT_FONT_SIZE);

  stageButton_ =
      new DockItem(stage_->getName(), &font, 16, 10, 80, DOCK_SIZE, 40);
  teamButtons_ = new DockItem*[numShips_];
  for (int x = 0; x < numShips_; x++) {
    teamButtons_[x] = new DockItem(ships_[x]->properties->name, &font, 16,
                                   0, 125 + (x * 35), DOCK_SIZE, 30);
  }

  sf::Shape** pauseShapes = new sf::Shape*[2];
  pauseShapes[0] = new sf::RectangleShape(sf::Vector2f(5, 20));
  pauseShapes[0]->move(-10, -10);
  pauseShapes[0]->setOutlineThickness(0);
  pauseShapes[1] = new sf::RectangleShape(sf::Vector2f(5, 20));
  pauseShapes[1]->move(5, -10);
  pauseShapes[1]->setOutlineThickness(0);
  pauseButton_ = new DockItem(pauseShapes, 2, 25, window->getSize().y - 100,
      50, 50, "Pause", &font, DOCK_BUTTON_FONT_SIZE, 21,
      window->getSize().y - 55, "Space", DOCK_SHORTCUT_FONT_SIZE);

  sf::Shape** playShapes = new sf::Shape*[1];
  sf::ConvexShape *playShape = new sf::ConvexShape(3);
  playShape->setPoint(0, sf::Vector2f(0, 0));
  playShape->setPoint(1, sf::Vector2f(0, 20));
  playShape->setPoint(2, sf::Vector2f(17, 10));
  playShape->setOutlineThickness(0);
  playShape->move(-7, -10);
  playShapes[0] = playShape;
  playButton_ = new DockItem(playShapes, 1, 25, window->getSize().y - 100, 50,
      50, "Resume", &font, DOCK_BUTTON_FONT_SIZE, 13, window->getSize().y - 55,
      "Space", DOCK_SHORTCUT_FONT_SIZE);

  sf::Shape** restartShapes = new sf::Shape*[4];
  restartShapes[0] = new sf::RectangleShape(sf::Vector2f(15, 2));
  restartShapes[0]->move(-5, -9);
  sf::ConvexShape *restartTriangle1 = new sf::ConvexShape(3);
  restartTriangle1->setPoint(0, sf::Vector2f(8, 0));
  restartTriangle1->setPoint(1, sf::Vector2f(8, 10));
  restartTriangle1->setPoint(2, sf::Vector2f(0, 5));
  restartTriangle1->setOutlineThickness(0);
  restartTriangle1->move(-13, -13);
  restartShapes[1] = restartTriangle1;
  restartShapes[2] = new sf::RectangleShape(sf::Vector2f(15, 2));
  restartShapes[2]->move(-10, 7);
  sf::ConvexShape *restartTriangle2 = new sf::ConvexShape(3);
  restartTriangle2->setPoint(0, sf::Vector2f(0, 0));
  restartTriangle2->setPoint(1, sf::Vector2f(0, 10));
  restartTriangle2->setPoint(2, sf::Vector2f(8, 5));
  restartTriangle2->setOutlineThickness(0);
  restartTriangle2->move(5, 3);
  restartShapes[3] = restartTriangle2;
  restartButton_ = new DockItem(restartShapes, 4, 85, window->getSize().y - 100,
      50, 50, "Restart", &font, DOCK_BUTTON_FONT_SIZE, 69,
      window->getSize().y - 55, "Back", DOCK_SHORTCUT_FONT_SIZE);
}

void GfxManager::reinitDockItems(sf::RenderWindow *window) {
  destroyDockItems();
  initDockItems(window);
}

void GfxManager::destroyBbGfx() {
  if (initialized_) {
    destroyDockItems();
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

void GfxManager::destroyDockItems() {
  if (newMatchButton_ != 0) {
    delete newMatchButton_;
    newMatchButton_ = 0;
  }
  if (stageButton_ != 0) {
    delete stageButton_;
    stageButton_ = 0;
  }
  if (teamButtons_ != 0) {
    for (int x = 0; x < numShips_; x++) {
      delete teamButtons_[x];
    }
    delete teamButtons_;
    teamButtons_ = 0;
  }
  if (pauseButton_ != 0) {
    delete pauseButton_;
  }
  if (playButton_ != 0) {
    delete playButton_;
  }
  if (restartButton_ != 0) {
    delete restartButton_;
  }
}

void GfxManager::setListener(GfxViewListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

// TODO: move all these args to class level?
void GfxManager::drawGame(sf::RenderWindow *window, Stage *stage, Ship **ships,
                          int numShips, int time, GfxEventHandler *gfxHandler,
                          bool paused, bool gameOver, char *winnerName) {
  if (showDock_) {
    drawDock(window, stage, paused);
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
  drawStageTexts(window, stage, paused, gameOver);
  if (gameOver) {
    drawGameOver(window, stage, winnerName);
  }
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
    reinitDockItems(window);
  } else {
    stageView.reset(sf::FloatRect(0, 0, viewWidth, viewHeight));
    stageView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window->setView(stageView);
  }
}

void GfxManager::processMouseClick(int x, int y) {
  if (listener_ != 0) {
    if (newMatchButton_->contains(x, y)) {
      listener_->onNewMatch();
    } else if (packageShipButton_->contains(x, y)) {
      listener_->onPackageShip();
    } else if (packageStageButton_->contains(x, y)) {
      listener_->onPackageStage();
    } else if (stageButton_->contains(x, y)) {
      listener_->onStageClick();
    } else if (pauseButton_->contains(x, y)) {
      listener_->onPauseUnpause();
    } else if (restartButton_->contains(x, y)) {
      listener_->onRestart();
    } else {
      for (int z = 0; z < numTeams_; z++) {
        if (teamButtons_[z]->contains(x, y)) {
          listener_->onTeamClick(teams_[z]->index);
        }
      }
    }
  }
}

void GfxManager::processMouseMoved(int x, int y) {
  newMatchButton_->setHighlights(x, y);
  packageShipButton_->setHighlights(x, y);
  packageStageButton_->setHighlights(x, y);
  stageButton_->setHighlights(x, y);
  pauseButton_->setHighlights(x, y);
  playButton_->setHighlights(x, y);
  restartButton_->setHighlights(x, y);
  for (int z = 0; z < numTeams_; z++) {
    teamButtons_[z]->setHighlights(x, y);
  }
}

void GfxManager::showKeyboardShortcuts() {
  newMatchButton_->showShortcut();
  packageShipButton_->showShortcut();
  packageStageButton_->showShortcut();
  stageButton_->showShortcut();
  pauseButton_->showShortcut();
  playButton_->showShortcut();
  restartButton_->showShortcut();
  for (int z = 0; z < numTeams_; z++) {
    teamButtons_[z]->showShortcut();
  }
}

void GfxManager::hideKeyboardShortcuts() {
  newMatchButton_->hideShortcut();
  packageShipButton_->hideShortcut();
  packageStageButton_->hideShortcut();
  stageButton_->hideShortcut();
  pauseButton_->hideShortcut();
  playButton_->hideShortcut();
  restartButton_->hideShortcut();
  for (int z = 0; z < numTeams_; z++) {
    teamButtons_[z]->hideShortcut();
  }
}

void GfxManager::drawWalls(sf::RenderWindow *window) {
  for (int x = 0; x < numWalls; x++) {
    window->draw(*(wallShapes[x]));
  }
}

void GfxManager::drawZones(sf::RenderWindow *window) {
  for (int x = 0; x < numZones; x++) {
    window->draw(*(zoneShapes[x]));
  }
}

void GfxManager::adjustTorpedoRayPoint(sf::RectangleShape *rayShape,
                                       double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f rayOffset = transform.transformPoint(torpedoRayPoint);
  rayShape->move(-rayOffset.x, -rayOffset.y);
}

void GfxManager::drawTorpedos(sf::RenderWindow *window, Stage *stage) {
  Torpedo **torpedos = stage->getTorpedos();
  int numTorpedos = stage->getTorpedoCount();
  for (int x = 0; x < numTorpedos; x++) {
    Torpedo *torpedo = torpedos[x];
    torpedoCircleShape.setPosition(adjustX(torpedo->x - TORPEDO_RADIUS),
        adjustY(torpedo->y - TORPEDO_RADIUS, TORPEDO_SIZE));
    window->draw(torpedoCircleShape);
    torpedoRay.setPosition(adjustX(torpedo->x), adjustY(torpedo->y));
    torpedoRay.setRotation(45);
    adjustTorpedoRayPoint(&torpedoRay, 45);
    window->draw(torpedoRay);
    torpedoRay.setRotation(135);
    torpedoRay.setPosition(adjustX(torpedo->x), adjustY(torpedo->y));
    adjustTorpedoRayPoint(&torpedoRay, 135);
    window->draw(torpedoRay);
  }
}

void GfxManager::drawTorpedoBlasts(sf::RenderWindow *window, int time,
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
      torpedoBlastShape.setPosition(adjustX(torpedoBlast->x - blastOffset),
          adjustY(torpedoBlast->y - blastOffset, blastOffset * 2));
      window->draw(torpedoBlastShape);
    }
  }
}

void GfxManager::adjustThrusterPosition(sf::RectangleShape *thrusterShape,
                                        double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f thrusterOffset = transform.transformPoint(thrusterPoint);
  thrusterShape->move(-thrusterOffset.x, -thrusterOffset.y);
}

void GfxManager::drawThrusters(sf::RenderWindow *window, Ship **ships,
                               int numShips) {
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
      thrusterShape.setPosition(adjustX(ship->x), adjustY(ship->y));
      adjustThrusterPosition(&thrusterShape, rotateAngle);
      window->draw(thrusterShape);
    }
  }
}

// TODO: combine some with adjustThrusterPosition
void GfxManager::adjustLaserPosition(sf::RectangleShape *laserShape,
                                     double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f laserOffset = transform.transformPoint(laserPoint);
  laserShape->move(-laserOffset.x, -laserOffset.y);
}

void GfxManager::drawLasers(sf::RenderWindow *window, Stage *stage) {
  Laser **lasers = stage->getLasers();
  int numLasers = stage->getLaserCount();
  for (int x = 0; x < numLasers; x++) {
    Laser *laser = lasers[x];
    double rotateAngle = toDegrees(-normalAbsoluteAngle(laser->heading));
    laserShape.setRotation(rotateAngle);
    laserShape.setPosition(adjustX(laser->x - laser->dx),
                           adjustY(laser->y - laser->dy));
    laserShape.setFillColor(laserColors[laser->shipIndex]);
    adjustLaserPosition(&laserShape, rotateAngle);
    window->draw(laserShape);
  }
}

void GfxManager::adjustShipDotPosition(sf::CircleShape *shipDotShape,
                                       double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f dotOffset = transform.transformPoint(shipDotPoint);
  shipDotShape->move(dotOffset.x, dotOffset.y);
}

void GfxManager::drawShips(sf::RenderWindow *window, Ship **ships, int numShips,
                           int time) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      shipShape.setOutlineColor(shipColors[x]);
      shipShape.setPosition(adjustX(ship->x - DRAW_SHIP_RADIUS),
                            adjustY(ship->y - DRAW_SHIP_RADIUS, DRAW_SHIP_RADIUS * 2));
      window->draw(shipShape);

      shipDotShape.setFillColor(laserColors[x]);
      for (int y = 0; y < 3; y++) {
        shipDotShape.setPosition(adjustX(ship->x - SHIP_DOT_RADIUS),
            adjustY(ship->y - SHIP_DOT_RADIUS, SHIP_DOT_RADIUS * 2));
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
      energyShape.setPosition(adjustX(ship->x - (ENERGY_LENGTH / 2)),
                              adjustY(ship->y - DRAW_SHIP_RADIUS - 8));
      energyShape.setScale(ship->energy / DEFAULT_ENERGY, 1);
      window->draw(energyShape);
    }
  }
}

void GfxManager::drawShipDeaths(sf::RenderWindow *window, int time,
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
        destroyedShape.setPosition(adjustX(shipDeath->x - thisRadius),
            adjustY(shipDeath->y - thisRadius, thisRadius * 2));
        window->draw(destroyedShape);
      }
    }
  }
}

void GfxManager::adjustLaserSparkPosition(sf::RectangleShape *sparkShape,
                                          double angle, int sparkTime) {
  sf::Transform transform;
  transform.rotate(angle);
  double scale = 1 + (((double) sparkTime) / 1.25);
  transform.scale(scale, scale);
  sf::Vector2f sparkOffset = transform.transformPoint(sparkPoint);
  sparkShape->move(sparkOffset.x, sparkOffset.y);
}

void GfxManager::drawLaserSparks(sf::RenderWindow *window, int time,
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
        sparkShape.setPosition(adjustX(ship->x), adjustY(ship->y));
        sparkShape.setRotation(laserHit->offsets[x]);
        adjustLaserSparkPosition(&sparkShape, laserHit->offsets[x], sparkTime);
        window->draw(sparkShape);
      }
    }
  }
}

void GfxManager::drawNames(sf::RenderWindow *window, Ship **ships,
                           int numShips) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->showName) {
      sf::Text text(ship->properties->name, font, 20);
      text.setColor(sf::Color::White);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(adjustX(ship->x - (textRect.width / 2)),
          adjustY(ship->y - SHIP_RADIUS - (ship->energyEnabled ? 10 : 4)));
      window->draw(text);
    }
  }
}

void GfxManager::drawStageTexts(sf::RenderWindow *window, Stage *stage,
                                bool paused, bool gameOver) {
  int numTexts = stage->getTextCount();
  if (numTexts > 0) {
    StageText **stageTexts = stage->getTexts();
    for (int x = 0; x < numTexts; x++) {
      StageText *stageText = stageTexts[x];
      sf::Text text(stageText->text, font, 28);
      text.setColor(sf::Color::White);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(adjustX(stageText->x),
                       adjustY(stageText->y + textRect.height));
      window->draw(text);
    }
    if (!paused && !gameOver) {
      stage->updateTextTimers();
    }
  }
}

void GfxManager::drawGameOver(sf::RenderWindow *window, Stage *stage,
                              char *winnerName) {
  sf::Text text;
  sf::RectangleShape borderShape;
  text.setFont(font);
  text.setCharacterSize(48);
  if (winnerName != 0 && strlen(winnerName) > 0) {
    text.setColor(sf::Color::Green);
    borderShape.setOutlineColor(sf::Color::Green);
    std::string winsText(winnerName);
    winsText.append(" wins!");
    text.setString(winsText);
  } else {
    text.setColor(sf::Color::Red);
    borderShape.setOutlineColor(sf::Color::Red);
    text.setString("Game Over");
  }
  sf::FloatRect textRect = text.getLocalBounds();
  text.setPosition(adjustX((stage->getWidth() - textRect.width) / 2),
                   adjustY(((stage->getHeight()) / 2) + 33));
  int borderWidth = textRect.width + 50;
  int borderHeight = textRect.height + 50;
  borderShape.setSize(sf::Vector2f(borderWidth, borderHeight));
  borderShape.setPosition(adjustX((stage->getWidth() - borderWidth) / 2),
                          adjustY((stage->getHeight() - borderHeight) / 2, borderHeight));
  borderShape.setOutlineThickness(4);
  borderShape.setFillColor(sf::Color::Black);
  window->draw(borderShape);
  window->draw(text);
}

void GfxManager::drawDock(sf::RenderWindow *window, Stage *stage, bool paused) {
  window->setView(dockView);
  drawDockItem(window, newMatchButton_);
  drawDockItem(window, packageShipButton_);
  drawDockItem(window, packageStageButton_);
  drawDockItem(window, stageButton_);
  drawDockItem(window, paused ? playButton_ : pauseButton_);
  drawDockItem(window, restartButton_);

  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    double teamEnergy = 0;
    double teamEnergyTotal = 0;
    bool showTeam = false;
    for (int y = 0; y < team->numShips; y++) {
      Ship *ship = ships_[team->firstShipIndex + y];
      showTeam = showTeam || ship->showName;
      if (ship->energyEnabled) {
        teamEnergy += ship->energy;
        teamEnergyTotal += DEFAULT_ENERGY;
      }
    }
    if (showTeam && team->shipsAlive > 0 && teamEnergyTotal > 0) {
      dockEnergyShape.setPosition(10, teamButtons_[x]->getBottom() + 20);
      dockEnergyShape.setScale(teamEnergy / teamEnergyTotal, 1);
      window->draw(dockEnergyShape);
    }
    window->draw(dockLineShape);
    if (showTeam) {
      drawDockItem(window, teamButtons_[x]);
    }
  }
  window->draw(dockMarginShape);
}

void GfxManager::drawDockItem(sf::RenderWindow *window, DockItem *dockItem) {
  sf::Drawable **drawables = dockItem->getDrawables();
  for (int x = 0; x < dockItem->getNumDrawables(); x++) {
    window->draw(*(drawables[x]));
  }
}

double GfxManager::adjustX(double x) {
  return STAGE_MARGIN + x;
}

double GfxManager::adjustY(double y, double height) {
  return windowHeight - STAGE_MARGIN - y - height;
}

double GfxManager::adjustY(double y) {
  return windowHeight - STAGE_MARGIN - y;
}
