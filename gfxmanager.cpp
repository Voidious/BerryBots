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

#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <string.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include "stage.h"
#include "dockshape.h"
#include "docktext.h"
#include "dockfader.h"
#include "gfxmanager.h"

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
  adjustingTps_ = false;
  dockTeamsViewHeight_ = 0;

  shipShape_.setRadius(DRAW_SHIP_RADIUS);
  shipDotShape_.setRadius(SHIP_DOT_RADIUS);
  shipDotPoint_ = sf::Vector2f(SHIP_DOT_POSITION, 0);
  destroyedShape_.setRadius(SHIP_DEATH_RADIUS);
  laserSparkShape_.setSize(sf::Vector2f(LASER_SPARK_LENGTH,
                                        LASER_SPARK_THICKNESS));
  laserSparkPoint_ = sf::Vector2f(DRAW_SHIP_RADIUS, -LASER_SPARK_THICKNESS / 2);
  laserShape_.setSize(sf::Vector2f(LASER_SPEED, LASER_THICKNESS));
  laserPoint_ = sf::Vector2f(0, LASER_THICKNESS / 2);
  torpedoSparkShape_.setRadius(TORPEDO_SPARK_RADIUS);
  torpedoSparkPoint_ = sf::Vector2f(DRAW_SHIP_RADIUS, -TORPEDO_SPARK_RADIUS);
  torpedoCircleShape_.setRadius(TORPEDO_RADIUS);
  torpedoRay_.setSize(sf::Vector2f(TORPEDO_SIZE * 2, 1));
  torpedoRayPoint_ = sf::Vector2f(TORPEDO_SIZE, .5);
  torpedoBlastShape_.setRadius(TORPEDO_BLAST_RADIUS);
  thrusterShape_.setSize(
      sf::Vector2f(DRAW_SHIP_RADIUS + THRUSTER_LENGTH, THRUSTER_THICKNESS));
  thrusterPoint_ = sf::Vector2f(0, THRUSTER_THICKNESS / 2);
  energyShape_.setSize(sf::Vector2f(ENERGY_LENGTH, ENERGY_THICKNESS));
  dockEnergyShape_.setSize(sf::Vector2f(DOCK_ENERGY_LENGTH, ENERGY_THICKNESS));
  // TODO: move these out to their own view
  dockLineShape_.setSize(sf::Vector2f(1, 8192));
  dockMarginShape_.setSize(sf::Vector2f(8, 8192));

  torpedoColor_ = TORPEDO_COLOR;
  blastColor_ = BLAST_COLOR;
  energyColor_ = ENERGY_COLOR;
  zoneColor_ = ZONE_COLOR;
  dockLineColor_ = DOCK_LINE_COLOR;
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
  windowHeight_ = viewHeight;
  teams_ = teams;
  numTeams_ = numTeams;
  ships_ = ships;
  numShips_ = numShips;
  stage_ = stage;
  
  if (!font_.loadFromFile(resourcePath + FONT_NAME)) {
    exit(EXIT_FAILURE);
  }

  initDockItems(window);
  shipColors_ = new sf::Color[numShips];
  shipDeathColors_ = new sf::Color[numShips];
  laserColors_ = new sf::Color[numShips];
  thrusterColors_ = new sf::Color[numShips];
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    shipColors_[x].r = ship->properties->shipR;
    shipColors_[x].g = ship->properties->shipG;
    shipColors_[x].b = ship->properties->shipB;
    shipDeathColors_[x].r = ship->properties->shipR * 3 / 4;
    shipDeathColors_[x].g = ship->properties->shipG * 3 / 4;
    shipDeathColors_[x].b = ship->properties->shipB * 3 / 4;
    laserColors_[x].r = ship->properties->laserR;
    laserColors_[x].g = ship->properties->laserG;
    laserColors_[x].b = ship->properties->laserB;
    thrusterColors_[x].r = ship->properties->thrusterR;
    thrusterColors_[x].g = ship->properties->thrusterG;
    thrusterColors_[x].b = ship->properties->thrusterB;
    shipColors_[x].a = thrusterColors_[x].a = 255;
  }
  shipDotOffsets_ = new int[numShips];
  shipDotDirections_ = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    shipDotOffsets_[x] = rand() % SHIP_DOT_FRAMES;
    shipDotDirections_[x] = (rand() % 10 < 5) ? true : false;
  }
  
  shipShape_.setOutlineThickness(SHIP_OUTLINE_THICKNESS);
  shipShape_.setFillColor(sf::Color::Black);
  shipDotShape_.setOutlineThickness(0);
  destroyedShape_.setOutlineThickness(2);
  destroyedShape_.setFillColor(sf::Color::Transparent);
  
  laserSparkShape_.setOutlineThickness(0);
  laserShape_.setOutlineThickness(0);
  
  torpedoCircleShape_.setOutlineColor(torpedoColor_);
  torpedoCircleShape_.setFillColor(torpedoColor_);
  torpedoRay_.setOutlineColor(torpedoColor_);
  torpedoRay_.setFillColor(torpedoColor_);
  torpedoBlastShape_.setOutlineColor(blastColor_);
  torpedoBlastShape_.setOutlineThickness(5);
  torpedoBlastShape_.setFillColor(sf::Color::Transparent);
  
  thrusterShape_.setOutlineThickness(0);
  energyShape_.setOutlineColor(energyColor_);
  energyShape_.setFillColor(energyColor_);
  dockEnergyShape_.setOutlineColor(energyColor_);
  dockEnergyShape_.setFillColor(energyColor_);
  dockLineShape_.setOutlineThickness(0);
  dockLineShape_.setFillColor(dockLineColor_);
  dockLineShape_.setPosition(DOCK_SIZE - 1, 0);
  dockMarginShape_.setOutlineThickness(0);
  dockMarginShape_.setFillColor(sf::Color::Black);
  dockMarginShape_.setPosition(DOCK_SIZE - 9, 0);
  
  numWalls_ = stage->getWallCount();
  wallShapes_ = new sf::RectangleShape*[numWalls_];
  Wall **walls = stage->getWalls();
  for (int x = 0; x < numWalls_; x++) {
    Wall *wall = walls[x];
    sf::RectangleShape *wallShape = new sf::RectangleShape(                                                          sf::Vector2f(wall->getWidth(), wall->getHeight()));
    wallShape->setPosition(adjustX(wall->getLeft()),
                           adjustY(wall->getBottom(), wall->getHeight()));
    wallShape->setOutlineColor(sf::Color::White);
    wallShape->setFillColor(sf::Color::White);
    wallShapes_[x] = wallShape;
  }
  
  numZones_ = stage->getZoneCount();
  zoneShapes_ = new sf::RectangleShape*[numZones_];
  Zone **zones = stage->getZones();
  for (int x = 0; x < numZones_; x++) {
    Zone *zone = zones[x];
    sf::RectangleShape *zoneShape = new sf::RectangleShape(                                                           sf::Vector2f(zone->getWidth(), zone->getHeight()));
    zoneShape->setPosition(adjustX(zone->getLeft()),
                           adjustY(zone->getBottom(), zone->getHeight()));
    zoneShape->setOutlineColor(zoneColor_);
    zoneShape->setFillColor(zoneColor_);
    zoneShapes_[x] = zoneShape;
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
  newMatchButton_ = new DockShape(newShapes, 2, 10, 10, 40, 40, "New Match",
      &font_, DOCK_BUTTON_FONT_SIZE, 26, 50, "N", DOCK_SHORTCUT_FONT_SIZE);

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
  packageShipButton_ = new DockShape(packageShipShapes, 4, 59, 10, 40, 40,
      "Package Ship", &font_, DOCK_BUTTON_FONT_SIZE, 17, 50, "P",
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
  packageStageButton_ = new DockShape(packageStageShapes, 5, 108, 10, 40, 40,
      "Package Stage", &font_, DOCK_BUTTON_FONT_SIZE, 9, 50, "T",
      DOCK_SHORTCUT_FONT_SIZE);

  stageButton_ = new DockText(stage_->getName(), &font_, SHIP_STAGE_FONT_SIZE,
                              10, 80, DOCK_SIZE - 10, 40);
  teamButtons_ = new DockText*[numTeams_];
  dockTeamsScrollPosition_ = 0;
  dockTeamsScrollBottom_ = 0;
  for (int x = 0; x < numTeams_; x++) {
    int teamTop = getShipDockTop(x);
    teamButtons_[x] = new DockText(teams_[x]->name, &font_,
        SHIP_STAGE_FONT_SIZE, 10, teamTop, DOCK_SIZE - 10, 30);
    dockTeamsScrollBottom_ = std::max(dockTeamsScrollBottom_,
                                      teamTop + 30 - DOCK_TOP_HEIGHT);
  }

  sf::Shape** pauseShapes = new sf::Shape*[2];
  pauseShapes[0] = new sf::RectangleShape(sf::Vector2f(5, 20));
  pauseShapes[0]->move(-10, -10);
  pauseShapes[0]->setOutlineThickness(0);
  pauseShapes[1] = new sf::RectangleShape(sf::Vector2f(5, 20));
  pauseShapes[1]->move(5, -10);
  pauseShapes[1]->setOutlineThickness(0);
  pauseButton_ = new DockShape(pauseShapes, 2, 25, window->getSize().y - 100,
      50, 50, "Pause", &font_, DOCK_BUTTON_FONT_SIZE, 21,
      window->getSize().y - 55, "Space", DOCK_SHORTCUT_FONT_SIZE);

  sf::Shape** playShapes = new sf::Shape*[1];
  sf::ConvexShape *playShape = new sf::ConvexShape(3);
  playShape->setPoint(0, sf::Vector2f(0, 0));
  playShape->setPoint(1, sf::Vector2f(0, 20));
  playShape->setPoint(2, sf::Vector2f(17, 10));
  playShape->setOutlineThickness(0);
  playShape->move(-7, -10);
  playShapes[0] = playShape;
  playButton_ = new DockShape(playShapes, 1, 25, window->getSize().y - 100, 50,
      50, "Resume", &font_, DOCK_BUTTON_FONT_SIZE, 13, window->getSize().y - 55,
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
  restartButton_ = new DockShape(restartShapes, 4, 85, window->getSize().y - 100,
      50, 50, "Restart", &font_, DOCK_BUTTON_FONT_SIZE, 69,
      window->getSize().y - 55, "Back", DOCK_SHORTCUT_FONT_SIZE);

  tpsFader_ = new DockFader(15, window->getSize().y - 140, DOCK_SIZE - 30, 40,
      "Speed", &font_, DOCK_BUTTON_FONT_SIZE, 49, window->getSize().y - 165);
}

void GfxManager::destroyBbGfx() {
  if (initialized_) {
    destroyDockItems();
    for (int x = 0; x < numWalls_; x++) {
      delete wallShapes_[x];
    }
    delete wallShapes_;
    for (int x = 0; x < numZones_; x++) {
      delete zoneShapes_[x];
    }
    delete zoneShapes_;
    delete shipColors_;
    delete shipDeathColors_;
    delete laserColors_;
    delete thrusterColors_;
    delete shipDotOffsets_;
    delete shipDotDirections_;
    initialized_ = false;
  }
}

void GfxManager::destroyDockItems() {
  if (newMatchButton_ != 0) {
    delete newMatchButton_;
    newMatchButton_ = 0;
  }
  if (packageShipButton_ != 0) {
    delete packageShipButton_;
    packageShipButton_ = 0;
  }
  if (packageStageButton_ != 0) {
    delete packageStageButton_;
    packageStageButton_ = 0;
  }
  if (stageButton_ != 0) {
    delete stageButton_;
    stageButton_ = 0;
  }
  if (teamButtons_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      delete teamButtons_[x];
    }
    delete teamButtons_;
    teamButtons_ = 0;
  }
  if (pauseButton_ != 0) {
    delete pauseButton_;
    pauseButton_ = 0;
  }
  if (playButton_ != 0) {
    delete playButton_;
    playButton_ = 0;
  }
  if (restartButton_ != 0) {
    delete restartButton_;
    restartButton_ = 0;
  }
  if (tpsFader_ != 0) {
    delete tpsFader_;
    tpsFader_ = 0;
  }
}

void GfxManager::setListener(GfxViewListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

void GfxManager::drawGame(sf::RenderWindow *window, Stage *stage, Ship **ships,
                          int numShips, int time, GfxEventHandler *gfxHandler,
                          bool paused, bool gameOver, const char *winnerName) {
  if (showDock_) {
    drawDock(window, stage, paused);
    window->setView(stageView_);
  }
  gfxHandler->removeShipDeaths(time - SHIP_DEATH_TIME);
  gfxHandler->removeLaserHits(time - LASER_SPARK_TIME);
  gfxHandler->removeTorpedoHits(time - TORPEDO_SPARK_TIME);
  gfxHandler->removeTorpedoBlasts(time - TORPEDO_BLAST_TIME);
  
  drawZones(window);
  drawTorpedos(window, stage);
  drawTorpedoBlasts(window, time, gfxHandler);
  drawWalls(window);
  drawShipDeaths(window, time, gfxHandler);
  drawLaserSparks(window, time, gfxHandler, ships);
  drawTorpedoSparks(window, time, gfxHandler, ships);
  drawThrusters(window, ships, numShips);
  drawNames(window, ships, numShips);
  drawLasers(window, stage);
  drawShips(window, ships, numShips, time);
  drawStageTexts(window, stage, time);
  if (gameOver) {
    drawGameOver(window, stage, winnerName);
  }
  drawUserGfxs(window, stage, time);
}

void GfxManager::increaseWindowSize(sf::RenderWindow *window, int viewWidth,
                                    int viewHeight) {
  adjustWindowScale(window, viewWidth, viewHeight, WINDOW_SIZE_STEP);
}

void GfxManager::decreaseWindowSize(sf::RenderWindow *window, int viewWidth,
                                    int viewHeight) {
  adjustWindowScale(window, viewWidth, viewHeight, -WINDOW_SIZE_STEP);
}

void GfxManager::defaultWindowSize(sf::RenderWindow *window, int viewWidth,
                                    int viewHeight) {
  unsigned int windowWidth = window->getSize().x;
  unsigned int dockSize = (showDock_ ? DOCK_SIZE : 0);
  double currentScale = ((double) windowWidth - dockSize) / viewWidth;
  adjustWindowScale(window, viewWidth, viewHeight, 1.0 - currentScale);
}

void GfxManager::adjustWindowScale(sf::RenderWindow *window, int viewWidth,
                                   int viewHeight, double scaleDelta) {
  unsigned int windowWidth = window->getSize().x;
  unsigned int windowHeight = window->getSize().y;
  unsigned int dockSize = (showDock_ ? DOCK_SIZE : 0);
  double currentScale = ((double) windowWidth - dockSize) / viewWidth;
  double newScale = currentScale + scaleDelta;
  windowWidth = (viewWidth * newScale) + dockSize;
  windowHeight = viewHeight * newScale;
  updateViews(window, viewWidth, viewHeight, windowWidth, windowHeight);
}

void GfxManager::initViews(sf::RenderWindow *window, unsigned int viewWidth,
                          unsigned int viewHeight) {
  resizeViews(window, viewWidth, viewHeight, true);
}

void GfxManager::onResize(sf::RenderWindow *window, unsigned int viewWidth,
                          unsigned int viewHeight) {
  resizeViews(window, viewWidth, viewHeight, false);
}

void GfxManager::resizeViews(sf::RenderWindow *window, unsigned int viewWidth,
                             unsigned int viewHeight, bool forceUpdate) {
  unsigned int oldWindowWidth = window->getSize().x;
  unsigned int oldWindowHeight = window->getSize().y;
  unsigned int dockSize = (showDock_ ? DOCK_SIZE : 0);

  double widthScale = ((double) oldWindowWidth - dockSize) / viewWidth;
  double heightScale = ((double) oldWindowHeight) / viewHeight;
  double scale = std::min(widthScale, heightScale);
  unsigned int windowWidth = round(scale * viewWidth) + dockSize;
  unsigned int windowHeight = round(scale * viewHeight);
  if (forceUpdate || windowWidth != oldWindowWidth
      || windowHeight != oldWindowHeight) {
    updateViews(window, viewWidth, viewHeight, windowWidth, windowHeight);
  }
}

// TODO: don't let user resize to smaller than dock top + dock bottom + some min
//       dock ships size
void GfxManager::updateViews(sf::RenderWindow *window, unsigned int viewWidth,
    unsigned int viewHeight, unsigned int windowWidth,
    unsigned int windowHeight) {
  window->setSize(sf::Vector2u(windowWidth, windowHeight));

  unsigned int dockSize = (showDock_ ? DOCK_SIZE : 0);
  if (showDock_) {
    unsigned int dockTeamsHeight =
        windowHeight - DOCK_TOP_HEIGHT - DOCK_BOTTOM_HEIGHT;
    dockTopView_.reset(sf::FloatRect(0, 0, dockSize, DOCK_TOP_HEIGHT));
    // TODO: preserve scroll position on resize
    dockTeamsScrollPosition_ = 0;
    dockTeamsView_.reset(sf::FloatRect(0, DOCK_TOP_HEIGHT,
                                       dockSize, dockTeamsHeight));
    dockBottomView_.reset(sf::FloatRect(0, windowHeight - DOCK_BOTTOM_HEIGHT,
                                     dockSize, DOCK_BOTTOM_HEIGHT));
    stageView_.reset(sf::FloatRect(0, 0, viewWidth, viewHeight));

    double dockViewportWidth = (((double) dockSize) / windowWidth);
    double stageViewportWidth = 1 - dockViewportWidth;
    double topViewportHeight = (((double) DOCK_TOP_HEIGHT) / windowHeight);
    double teamsViewportHeight = (((double) dockTeamsHeight) / windowHeight);
    double bottomViewportHeight = 1.0f - topViewportHeight - teamsViewportHeight;
    dockTopView_.setViewport(
        sf::FloatRect(0.f, 0.f, dockViewportWidth, topViewportHeight));
    dockTeamsView_.setViewport(sf::FloatRect(
        0.f, topViewportHeight, dockViewportWidth, teamsViewportHeight));
    dockBottomView_.setViewport(sf::FloatRect(0.f,
        topViewportHeight + teamsViewportHeight, dockViewportWidth,
        bottomViewportHeight));
    stageView_.setViewport(sf::FloatRect(dockViewportWidth, 0.f,
                                        stageViewportWidth, 1.f));
    dockTeamsViewHeight_ = dockTeamsHeight;
  } else {
    stageView_.reset(sf::FloatRect(0, 0, viewWidth, viewHeight));
    stageView_.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window->setView(stageView_);
  }
}

void GfxManager::processMouseDown(int x, int y) {
  if (tpsFader_->contains(x, y)) {
    adjustingTps_ = true;
    tpsFader_->setKnob(x);
  }

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
    } else if (tpsFader_->contains(x, y)) {
      listener_->onTpsChange(tpsFader_->getVolume());
    } else {
      for (int z = 0; z < numTeams_; z++) {
        if (!teamButtons_[z]->hidden()
            && teamButtons_[z]->contains(x, y + dockTeamsScrollPosition_)) {
          listener_->onTeamClick(teams_[z]->index);
        }
      }
    }
  }
}

void GfxManager::processMouseUp(int x, int y) {
  adjustingTps_ = false;
  tpsFader_->setHighlights(x, y);
}

void GfxManager::processMouseMoved(int x, int y) {
  if (adjustingTps_) {
    tpsFader_->setKnob(x);
    listener_->onTpsChange(tpsFader_->getVolume());
  } else {
    stageButton_->setHighlights(x, y);
    for (int z = 0; z < numTeams_; z++) {
      teamButtons_[z]->setHighlights(x, y + dockTeamsScrollPosition_);
    }

    newMatchButton_->setHighlights(x, y);
    packageShipButton_->setHighlights(x, y);
    packageStageButton_->setHighlights(x, y);
    pauseButton_->setHighlights(x, y);
    playButton_->setHighlights(x, y);
    restartButton_->setHighlights(x, y);
    tpsFader_->setHighlights(x, y);
  }
}

void GfxManager::processMouseWheel(int x, int y, int delta) {
  delta = limit(-dockTeamsScrollPosition_, delta * -35,
      std::max(dockTeamsViewHeight_, dockTeamsScrollBottom_)
          - dockTeamsScrollPosition_ - dockTeamsViewHeight_);
  dockTeamsView_.move(0, delta);
  dockTeamsScrollPosition_ += delta;
  processMouseMoved(x, y);
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
    if (!teamButtons_[z]->hidden()) {
      teamButtons_[z]->showShortcut();
    }
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
  for (int x = 0; x < numWalls_; x++) {
    window->draw(*(wallShapes_[x]));
  }
}

void GfxManager::drawZones(sf::RenderWindow *window) {
  for (int x = 0; x < numZones_; x++) {
    window->draw(*(zoneShapes_[x]));
  }
}

void GfxManager::adjustTorpedoRayPoint(sf::RectangleShape *rayShape,
                                       double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f rayOffset = transform.transformPoint(torpedoRayPoint_);
  rayShape->move(-rayOffset.x, -rayOffset.y);
}

void GfxManager::drawTorpedos(sf::RenderWindow *window, Stage *stage) {
  Torpedo **torpedos = stage->getTorpedos();
  int numTorpedos = stage->getTorpedoCount();
  for (int x = 0; x < numTorpedos; x++) {
    Torpedo *torpedo = torpedos[x];
    torpedoCircleShape_.setPosition(adjustX(torpedo->x - TORPEDO_RADIUS),
        adjustY(torpedo->y - TORPEDO_RADIUS, TORPEDO_SIZE));
    window->draw(torpedoCircleShape_);
    torpedoRay_.setPosition(adjustX(torpedo->x), adjustY(torpedo->y));
    torpedoRay_.setRotation(45);
    adjustTorpedoRayPoint(&torpedoRay_, 45);
    window->draw(torpedoRay_);
    torpedoRay_.setRotation(135);
    torpedoRay_.setPosition(adjustX(torpedo->x), adjustY(torpedo->y));
    adjustTorpedoRayPoint(&torpedoRay_, 135);
    window->draw(torpedoRay_);
  }
}

void GfxManager::drawTorpedoBlasts(sf::RenderWindow *window, int time,
                                   GfxEventHandler *gfxHandler) {
  TorpedoBlastGraphic **torpedoBlasts = gfxHandler->getTorpedoBlasts();
  int numTorpedoBlasts = gfxHandler->getTorpedoBlastCount();
  
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
    torpedoBlastShape_.setRadius(blastScale * TORPEDO_BLAST_RADIUS);
    torpedoBlastShape_.setPosition(adjustX(torpedoBlast->x - blastOffset),
        adjustY(torpedoBlast->y - blastOffset, blastOffset * 2));
    window->draw(torpedoBlastShape_);
  }
}

void GfxManager::adjustThrusterPosition(sf::RectangleShape *thrusterShape,
                                        double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f thrusterOffset = transform.transformPoint(thrusterPoint_);
  thrusterShape->move(-thrusterOffset.x, -thrusterOffset.y);
}

void GfxManager::drawThrusters(sf::RenderWindow *window, Ship **ships,
                               int numShips) {
  for (int z = 0; z < numShips; z++) {
    Ship *ship = ships[z];
    if (ship->alive && ship->thrusterForce > 0) {
      double forceFactor = ship->thrusterForce / MAX_THRUSTER_FORCE;
      double lengthScale = THRUSTER_ZERO + (forceFactor * (1 - THRUSTER_ZERO));
      thrusterShape_.setFillColor(thrusterColors_[z]);
      thrusterShape_.setScale(lengthScale, lengthScale);
      double rotateAngle =
          toDegrees(-normalAbsoluteAngle(ship->thrusterAngle + M_PI));
      thrusterShape_.setRotation(rotateAngle);
      thrusterShape_.setPosition(adjustX(ship->x), adjustY(ship->y));
      adjustThrusterPosition(&thrusterShape_, rotateAngle);
      window->draw(thrusterShape_);
    }
  }
}

// TODO: combine some with adjustThrusterPosition
void GfxManager::adjustLaserPosition(sf::RectangleShape *laserShape,
                                     double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f laserOffset = transform.transformPoint(laserPoint_);
  laserShape->move(-laserOffset.x, -laserOffset.y);
}

void GfxManager::drawLasers(sf::RenderWindow *window, Stage *stage) {
  Laser **lasers = stage->getLasers();
  int numLasers = stage->getLaserCount();
  for (int x = 0; x < numLasers; x++) {
    Laser *laser = lasers[x];
    double rotateAngle = toDegrees(-normalAbsoluteAngle(laser->heading));
    laserShape_.setRotation(rotateAngle);
    laserShape_.setPosition(adjustX(laser->x - laser->dx),
                            adjustY(laser->y - laser->dy));
    laserShape_.setFillColor(laserColors_[laser->shipIndex]);
    adjustLaserPosition(&laserShape_, rotateAngle);
    window->draw(laserShape_);
  }
}

void GfxManager::adjustShipDotPosition(sf::CircleShape *shipDotShape,
                                       double angle) {
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f dotOffset = transform.transformPoint(shipDotPoint_);
  shipDotShape->move(dotOffset.x, dotOffset.y);
}

void GfxManager::drawShips(sf::RenderWindow *window, Ship **ships, int numShips,
                           int time) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      shipShape_.setOutlineColor(shipColors_[x]);
      shipShape_.setPosition(adjustX(ship->x - DRAW_SHIP_RADIUS),
                             adjustY(ship->y - DRAW_SHIP_RADIUS, DRAW_SHIP_RADIUS * 2));
      window->draw(shipShape_);

      shipDotShape_.setFillColor(laserColors_[x]);
      for (int y = 0; y < 3; y++) {
        shipDotShape_.setPosition(adjustX(ship->x - SHIP_DOT_RADIUS),
            adjustY(ship->y - SHIP_DOT_RADIUS, SHIP_DOT_RADIUS * 2));
        double angle = (M_PI * 2 * y / 3)
            + ((((double) ((time + shipDotOffsets_[x]) % SHIP_DOT_FRAMES))
                / SHIP_DOT_FRAMES) * (shipDotDirections_[x] ? 2 : -2) * M_PI);
        adjustShipDotPosition(&shipDotShape_, toDegrees(angle));
        window->draw(shipDotShape_);
      }
    }
  }
  
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->energyEnabled) {
      energyShape_.setPosition(adjustX(ship->x - (ENERGY_LENGTH / 2)),
                               adjustY(ship->y - DRAW_SHIP_RADIUS - 8));
      energyShape_.setScale(ship->energy / DEFAULT_ENERGY, 1);
      window->draw(energyShape_);
    }
  }
}

void GfxManager::drawShipDeaths(sf::RenderWindow *window, int time,
                                GfxEventHandler *gfxHandler) {
  ShipDeathGraphic **shipDeaths = gfxHandler->getShipDeaths();
  int numShipDeaths = gfxHandler->getShipDeathCount();

  for (int x = 0; x < numShipDeaths; x++) {
    ShipDeathGraphic *shipDeath = shipDeaths[x];
    int deathTime = (time - shipDeath->time) / SHIP_DEATH_FRAME_LENGTH;
    destroyedShape_.setOutlineColor(shipDeathColors_[shipDeath->shipIndex]);
    for (int y = std::max(0, deathTime - 3); y < deathTime; y++) {
      double thisRadius = SHIP_DEATH_RADIUS * (1 + y);
      destroyedShape_.setRadius(thisRadius);
      destroyedShape_.setPosition(adjustX(shipDeath->x - thisRadius),
          adjustY(shipDeath->y - thisRadius, thisRadius * 2));
      window->draw(destroyedShape_);
    }
  }
}

void GfxManager::adjustLaserSparkPosition(sf::RectangleShape *sparkShape,
                                          double angle, int sparkTime) {
  sf::Transform transform;
  transform.rotate(angle);
  double scale = 1 + (((double) sparkTime) / 1.25);
  transform.scale(scale, scale);
  sf::Vector2f sparkOffset = transform.transformPoint(laserSparkPoint_);
  sparkShape->move(sparkOffset.x, sparkOffset.y);
}

void GfxManager::drawLaserSparks(sf::RenderWindow *window, int time,
                                 GfxEventHandler *gfxHandler, Ship **ships) {
  LaserHitShipGraphic **laserHits = gfxHandler->getLaserHits();
  int numLaserHits = gfxHandler->getLaserHitCount();
  
  for (int x = 0; x < numLaserHits; x++) {
    LaserHitShipGraphic *laserHit = laserHits[x];
    int sparkTime = (time - laserHit->time);
    double dx = sparkTime * laserHit->dx;
    double dy = sparkTime * laserHit->dy;
    laserSparkShape_.setFillColor(laserColors_[laserHit->srcShipIndex]);
    for (int x = 0; x < 4; x++) {
      laserSparkShape_.setPosition(adjustX(laserHit->x + dx),
                                   adjustY(laserHit->y + dy));
      laserSparkShape_.setRotation(laserHit->offsets[x]);
      adjustLaserSparkPosition(&laserSparkShape_, laserHit->offsets[x],
                               sparkTime);
      window->draw(laserSparkShape_);
    }
  }
}

void GfxManager::adjustTorpedoSparkPosition(sf::CircleShape *sparkShape,
    double angle, int sparkTime, int sparkSpeed) {
  sf::Transform transform;
  transform.rotate(angle);
  double scale = 1 + ((((double) sparkTime) / 3) * sparkSpeed / 100);
  transform.scale(scale, scale);
  sf::Vector2f sparkOffset = transform.transformPoint(torpedoSparkPoint_);
  sparkShape->move(sparkOffset.x, sparkOffset.y);
}

void GfxManager::drawTorpedoSparks(sf::RenderWindow *window, int time,
                                   GfxEventHandler *gfxHandler, Ship **ships) {
  TorpedoHitShipGraphic **torpedoHits = gfxHandler->getTorpedoHits();
  int numTorpedoHits = gfxHandler->getTorpedoHitCount();
  
  for (int x = 0; x < numTorpedoHits; x++) {
    TorpedoHitShipGraphic *torpedoHit = torpedoHits[x];
    int sparkTime = (time - torpedoHit->time);
    double dx = (sparkTime * torpedoHit->dx);
    double dy = (sparkTime * torpedoHit->dy);
    torpedoSparkShape_.setFillColor(shipColors_[torpedoHit->hitShipIndex]);
    for (int x = 0; x < torpedoHit->numTorpedoSparks; x++) {
      torpedoSparkShape_.setPosition(adjustX(torpedoHit->x + dx),
                                     adjustY(torpedoHit->y + dy));
      torpedoSparkShape_.setRotation(torpedoHit->offsets[x]);
      adjustTorpedoSparkPosition(&torpedoSparkShape_, torpedoHit->offsets[x],
                                 sparkTime, torpedoHit->speeds[x]);
      window->draw(torpedoSparkShape_);
    }
  }
}

void GfxManager::drawNames(sf::RenderWindow *window, Ship **ships,
                           int numShips) {
  for (int x = 0; x < numShips; x++) {
    Ship *ship = ships[x];
    if (ship->alive && ship->showName) {
      sf::Text text(ship->properties->name, font_, 20);
      text.setColor(sf::Color::White);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(adjustX(ship->x - (textRect.width / 2)),
          adjustY(ship->y - SHIP_RADIUS - (ship->energyEnabled ? 10 : 4)));
      window->draw(text);
    }
  }
}

void GfxManager::drawStageTexts(sf::RenderWindow *window, Stage *stage,
                                int time) {
  stage->clearStaleStageTexts(time);
  int numTexts = stage->getStageTextCount();
  if (numTexts > 0) {
    StageText **stageTexts = stage->getStageTexts();
    for (int x = 0; x < numTexts; x++) {
      StageText *stageText = stageTexts[x];
      sf::Text text(stageText->text, font_,
          limit(MIN_TEXT_FONT_SIZE, stageText->fontSize, MAX_TEXT_FONT_SIZE));
      text.setColor(sf::Color(stageText->textR, stageText->textG,
                              stageText->textB, stageText->textA));
      sf::FloatRect textRect = text.getLocalBounds();
      text.setPosition(adjustX(stageText->x),
                       adjustY(stageText->y + round(textRect.height * 1.5)));
      window->draw(text);
    }
  }
}

void GfxManager::adjustUserGfxRectanglePosition(
    sf::RectangleShape *rectangleShape, double angle) {
  int centerX = rectangleShape->getSize().x / 2;
  int centerY = rectangleShape->getSize().y / 2;
  sf::Vector2f rectanglePoint(centerX, centerY);
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f rectangleOffset = transform.transformPoint(rectanglePoint);
  rectangleShape->move(centerX - rectangleOffset.x,
                       centerY - rectangleOffset.y);
}

void GfxManager::adjustUserGfxLinePosition(sf::RectangleShape *rectangleShape,
                                           double angle) {
  sf::Vector2f rectanglePoint(0, rectangleShape->getSize().y / 2);
  sf::Transform transform;
  transform.rotate(angle);
  sf::Vector2f rectangleOffset = transform.transformPoint(rectanglePoint);
  rectangleShape->move(-rectangleOffset.x, -rectangleOffset.y);
}

void GfxManager::drawUserGfxs(sf::RenderWindow *window, Stage *stage,
                              int time) {
  for (int x = 0; x < numTeams_; x++) {
    if (teams_[x]->gfxEnabled) {
      drawUserGfxRectangles(window, stage->getShipGfxRectangles(x),
                            stage->getShipGfxRectangleCount(x));
      drawUserGfxLines(window, stage->getShipGfxLines(x),
                       stage->getShipGfxLineCount(x));
      drawUserGfxCircles(window, stage->getShipGfxCircles(x),
                         stage->getShipGfxCircleCount(x));
      drawUserGfxTexts(window, stage->getShipGfxTexts(x),
                         stage->getShipGfxTextCount(x));
    }
  }

  if (stage->getGfxEnabled()) {
    drawUserGfxRectangles(window, stage->getStageGfxRectangles(),
                          stage->getStageGfxRectangleCount());
    drawUserGfxLines(window, stage->getStageGfxLines(),
                     stage->getStageGfxLineCount());
    drawUserGfxCircles(window, stage->getStageGfxCircles(),
                       stage->getStageGfxCircleCount());
    drawUserGfxTexts(window, stage->getStageGfxTexts(),
                     stage->getStageGfxTextCount());
  }
}

void GfxManager::drawUserGfxRectangles(sf::RenderWindow *window,
    UserGfxRectangle** gfxRectangles, int numRectangles) {
  for (int y = 0; y < numRectangles; y++) {
    UserGfxRectangle *gfxRectangle = gfxRectangles[y];
    sf::RectangleShape rectangle(sf::Vector2f(gfxRectangle->width,
                                              gfxRectangle->height));
    double rotateAngle =
        toDegrees(-normalAbsoluteAngle(gfxRectangle->rotation));
    rectangle.setRotation(rotateAngle);
    rectangle.setPosition(adjustX(gfxRectangle->left),
                          adjustY(gfxRectangle->bottom, gfxRectangle->height));
    rectangle.setFillColor(sf::Color(gfxRectangle->fillR, gfxRectangle->fillG,
                                     gfxRectangle->fillB, gfxRectangle->fillA));
    rectangle.setOutlineThickness(gfxRectangle->outlineThickness);
    rectangle.setOutlineColor(sf::Color(gfxRectangle->outlineR,
        gfxRectangle->outlineG, gfxRectangle->outlineB,
        gfxRectangle->outlineA));
    adjustUserGfxRectanglePosition(&rectangle, rotateAngle);
    window->draw(rectangle);
  }
}

void GfxManager::drawUserGfxLines(sf::RenderWindow *window,
                                  UserGfxLine** gfxLines, int numLines) {
  for (int y = 0; y < numLines; y++) {
    UserGfxLine *gfxLine = gfxLines[y];
    sf::RectangleShape line(sf::Vector2f(gfxLine->length, gfxLine->thickness));
    double rotateAngle = toDegrees(-normalAbsoluteAngle(gfxLine->angle));
    line.setRotation(rotateAngle);
    line.setPosition(adjustX(gfxLine->x), adjustY(gfxLine->y));
    line.setFillColor(sf::Color(gfxLine->fillR, gfxLine->fillG, gfxLine->fillB,
                                gfxLine->fillA));
    line.setOutlineThickness(gfxLine->outlineThickness);
    line.setOutlineColor(sf::Color(gfxLine->outlineR, gfxLine->outlineG,
                                   gfxLine->outlineB, gfxLine->outlineA));
    adjustUserGfxLinePosition(&line, rotateAngle);
    window->draw(line);
  }
}

void GfxManager::drawUserGfxCircles(sf::RenderWindow *window,
    UserGfxCircle** gfxCircles, int numCircles) {
  for (int y = 0; y < numCircles; y++) {
    UserGfxCircle *gfxCircle = gfxCircles[y];
    sf::CircleShape circle(gfxCircle->radius);
    circle.setPosition(adjustX(gfxCircle->x) - gfxCircle->radius,
        adjustY(gfxCircle->y - gfxCircle->radius, gfxCircle->radius * 2));
    circle.setFillColor(sf::Color(gfxCircle->fillR, gfxCircle->fillG,
                                  gfxCircle->fillB, gfxCircle->fillA));
    circle.setOutlineThickness(gfxCircle->outlineThickness);
    circle.setOutlineColor(sf::Color(gfxCircle->outlineR, gfxCircle->outlineG,
                                     gfxCircle->outlineB, gfxCircle->outlineA));
    window->draw(circle);
  }
}

void GfxManager::drawUserGfxTexts(sf::RenderWindow *window,
                                  UserGfxText** gfxTexts, int numTexts) {
  for (int y = 0; y < numTexts; y++) {
    UserGfxText *gfxText = gfxTexts[y];
    sf::Text text(gfxText->text, font_,
        limit(MIN_TEXT_FONT_SIZE, gfxText->fontSize, MAX_TEXT_FONT_SIZE));
    text.setColor(sf::Color(gfxText->textR, gfxText->textG, gfxText->textB,
                            gfxText->textA));
    sf::FloatRect textRect = text.getLocalBounds();
    text.setPosition(adjustX(gfxText->x),
                     adjustY(gfxText->y + round(textRect.height * 1.5)));
    window->draw(text);
  }
}

void GfxManager::drawGameOver(sf::RenderWindow *window, Stage *stage,
                              const char *winnerName) {
  sf::Text text;
  sf::RectangleShape borderShape;
  text.setFont(font_);
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
  window->setView(dockTopView_);
  drawDockItem(window, newMatchButton_);
  drawDockItem(window, packageShipButton_);
  drawDockItem(window, packageStageButton_);
  drawDockItem(window, stageButton_);
  window->draw(dockLineShape_);
  window->draw(dockMarginShape_);

  window->setView(dockBottomView_);
  pauseButton_->setTop(window->getSize().y - 85, window->getSize().y - 40);
  playButton_->setTop(window->getSize().y - 85, window->getSize().y - 40);
  restartButton_->setTop(window->getSize().y - 85, window->getSize().y - 40);
  tpsFader_->setTop(window->getSize().y - 125, window->getSize().y - 150);
  drawDockItem(window, paused ? playButton_ : pauseButton_);
  drawDockItem(window, restartButton_);
  drawDockItem(window, tpsFader_);
  window->draw(dockLineShape_);
  window->draw(dockMarginShape_);

  window->setView(dockTeamsView_);
  int dockIndex = 0;
  dockTeamsScrollBottom_ = 0;
  for (int x = 0; x < numTeams_; x++) {
    int teamTop = getShipDockTop(x);
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
    teamButtons_[x]->setDisabled(team->disabled);
    teamButtons_[x]->setErrored(team->errored);
    teamButtons_[x]->setHidden(!showTeam);
    if (showTeam) {
      teamButtons_[x]->setTop(getShipDockTop(dockIndex++));
      if (team->shipsAlive > 0 && teamEnergyTotal > 0) {
        dockEnergyShape_.setPosition(10, teamButtons_[x]->getTop() + 20);
        dockEnergyShape_.setScale(std::max(0., teamEnergy) / teamEnergyTotal, 1);
        window->draw(dockEnergyShape_);
      }
      dockTeamsScrollBottom_ = std::max(dockTeamsScrollBottom_,
                                        teamTop + 30 - DOCK_TOP_HEIGHT);
      drawDockItem(window, teamButtons_[x]);
    }
  }
  if (dockTeamsViewHeight_ < dockTeamsScrollBottom_) {
    double scrollPercentage = ((double) dockTeamsScrollPosition_)
        / (dockTeamsScrollBottom_ - dockTeamsViewHeight_);
    sf::CircleShape dockScrollCircle(3);
    dockScrollCircle.setPosition(DOCK_SIZE - 20,
        (scrollPercentage * (dockTeamsViewHeight_ - 20)) + DOCK_TOP_HEIGHT + 10
            + dockTeamsScrollPosition_);
    dockScrollCircle.setOutlineThickness(0);
    dockScrollCircle.setFillColor(sf::Color::Green);
    window->draw(dockScrollCircle);
  }
  window->draw(dockLineShape_);
  window->draw(dockMarginShape_);
}

void GfxManager::drawDockItem(sf::RenderWindow *window, DockItem *dockItem) {
  sf::Drawable **drawables = dockItem->getDrawables();
  for (int x = 0; x < dockItem->getNumDrawables(); x++) {
    window->draw(*(drawables[x]));
  }
}

int GfxManager::getShipDockTop(int index) {
  return DOCK_TOP_HEIGHT + 10 + (index * 35);
}

double GfxManager::adjustX(double x) {
  return STAGE_MARGIN + x;
}

double GfxManager::adjustY(double y, double height) {
  return windowHeight_ - STAGE_MARGIN - y - height;
}

double GfxManager::adjustY(double y) {
  return windowHeight_ - STAGE_MARGIN - y;
}
