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

#ifndef GFX_MANAGER_H
#define GFX_MANAGER_H

#include <SFML/Graphics.hpp>
#include "stage.h"
#include "bbutil.h"
#include "gfxeventhandler.h"
#include "dockitem.h"
#include "dockfader.h"

#define DRAW_SHIP_RADIUS        (SHIP_RADIUS - .7)
#define SHIP_OUTLINE_THICKNESS  1.7

#define STAGE_MARGIN             25
#define DOCK_SIZE                159
#define SHIP_DOT_POSITION        DRAW_SHIP_RADIUS * .45
#define SHIP_DOT_RADIUS          DRAW_SHIP_RADIUS * .2
#define SHIP_DOT_FRAMES          240
#define LASER_THICKNESS          2
#define THRUSTER_THICKNESS       6
#define THRUSTER_LENGTH          12
#define ENERGY_THICKNESS         2
#define ENERGY_LENGTH            (DRAW_SHIP_RADIUS * 4)
#define THRUSTER_ZERO            (DRAW_SHIP_RADIUS + 1.0) / (DRAW_SHIP_RADIUS + THRUSTER_LENGTH)
#define TORPEDO_RADIUS           4
#define TORPEDO_SIZE             TORPEDO_RADIUS * 2
#define SHIP_DEATH_FRAMES        16
#define SHIP_DEATH_FRAME_LENGTH  2
#define SHIP_DEATH_TIME          (SHIP_DEATH_FRAMES * SHIP_DEATH_FRAME_LENGTH)
#define SHIP_DEATH_RADIUS        6
#define LASER_SPARK_LENGTH       8
#define LASER_SPARK_THICKNESS    1.5
#define LASER_SPARK_FRAMES       8
#define LASER_SPARK_TIME         LASER_SPARK_FRAMES
#define TORPEDO_BLAST_FRAMES     16
#define TORPEDO_BLAST_TIME       TORPEDO_BLAST_FRAMES
#define FONT_NAME                "Questrial-Regular.ttf"

#define NEW_MATCH_DOCK_TEXT      "New Match..."
#define DOCK_ENERGY_LENGTH       DOCK_SIZE - 40
#define DOCK_BUTTON_FONT_SIZE     20
#define DOCK_SHORTCUT_FONT_SIZE   16

class GfxViewListener {
  public:
    virtual void onNewMatch() = 0;
    virtual void onPackageShip() = 0;
    virtual void onPackageStage() = 0;
    virtual void onStageClick() = 0;
    virtual void onTeamClick(int teamIndex) = 0;
    virtual void onPauseUnpause() = 0;
    virtual void onRestart() = 0;
    virtual void onTpsChange(double tpsFactor) = 0;
    virtual ~GfxViewListener() {};
};

class GfxManager {
  bool showDock_;
  DockItem *newMatchButton_;
  DockItem *packageShipButton_;
  DockItem *packageStageButton_;
  DockItem *stageButton_;
  DockItem **teamButtons_;
  DockItem *pauseButton_;
  DockItem *playButton_;
  DockItem *restartButton_;
  DockFader *tpsFader_;
  GfxViewListener *listener_;
  Team **teams_;
  int numTeams_;
  Ship **ships_;
  int numShips_;
  Stage *stage_;
  bool initialized_;
  bool adjustingTps_;

  public:
    GfxManager(bool showDock);
    ~GfxManager();
    void initBbGfx(sf::RenderWindow *window, unsigned int viewHeight,
                   Stage *stage, Team **teams, int numTeams, Ship **ships,
                   int numShips, std::string resourcePath);
    void destroyBbGfx();
    void setListener(GfxViewListener *listener);
    void drawGame(sf::RenderWindow *window, Stage *stage, Ship **ships,
                  int numShips, int time, GfxEventHandler *gfxHandler,
                  bool paused, bool gameOver, char *winnerName);
    void updateView(sf::RenderWindow *window, unsigned int viewWidth,
                    unsigned int viewHeight);
    void processMouseDown(int x, int y);
    void processMouseUp(int x, int y);
    void processMouseMoved(int x, int y);
    void showKeyboardShortcuts();
    void hideKeyboardShortcuts();
  private:
    void initDockItems(sf::RenderWindow *window);
    void reinitDockItems(sf::RenderWindow *window);
    void destroyDockItems();
    void drawWalls(sf::RenderWindow *window);
    void drawZones(sf::RenderWindow *window);
    void adjustTorpedoRayPoint(sf::RectangleShape *rayShape, double angle);
    void drawTorpedos(sf::RenderWindow *window, Stage *stage);
    void drawTorpedoBlasts(sf::RenderWindow *window, int time,
                           GfxEventHandler *gfxHandler);
    void adjustThrusterPosition(sf::RectangleShape *thrusterShape,
                                double angle);
    void drawThrusters(sf::RenderWindow *window, Ship **ships, int numShips);
    void adjustLaserPosition(sf::RectangleShape *laserShape, double angle);
    void drawLasers(sf::RenderWindow *window, Stage *stage);
    void adjustShipDotPosition(sf::CircleShape *shipDotShape, double angle);
    void drawShips(sf::RenderWindow *window, Ship **ships, int numShips,
                   int time);
    void drawShipDeaths(sf::RenderWindow *window, int time,
                        GfxEventHandler *gfxHandler);
    void adjustLaserSparkPosition(sf::RectangleShape *sparkShape, double angle,
                                  int sparkTime);
    void drawLaserSparks(sf::RenderWindow *window, int time,
                         GfxEventHandler *gfxHandler, Ship **ships);
    void drawNames(sf::RenderWindow *window, Ship **ships, int numShips);
    void drawStageTexts(sf::RenderWindow *window, Stage *stage, bool paused,
                        bool gameOver, int time);
    void drawDock(sf::RenderWindow *window, Stage *stage, bool paused);
    void drawDockItem(sf::RenderWindow *window, DockItem *dockItem);
    void drawGameOver(sf::RenderWindow *window, Stage *stage, char *winnerName);
    double adjustX(double x);
    double adjustY(double x, double height);
    double adjustY(double x);
};

#endif
