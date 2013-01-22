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

#define STAGE_MARGIN             25
#define DOCK_SIZE                150
#define SHIP_DOT_POSITION        SHIP_RADIUS * .45
#define SHIP_DOT_RADIUS          SHIP_RADIUS * .2
#define SHIP_DOT_FRAMES          240
#define LASER_THICKNESS          2
#define THRUSTER_THICKNESS       6
#define THRUSTER_LENGTH          SHIP_SIZE
#define ENERGY_THICKNESS         2
#define ENERGY_LENGTH            (SHIP_SIZE * 2)
#define THRUSTER_ZERO            (SHIP_RADIUS + 1.0) / (SHIP_RADIUS + THRUSTER_LENGTH)
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

#define NEW_MATCH_TEXT           "New Match..."
#define DOCK_ENERGY_LENGTH       DOCK_SIZE - 40

class GfxViewListener {
  public:
    virtual void onNewMatch() = 0;
    virtual void onStageClick() = 0;
    virtual void onTeamClick(int teamIndex) = 0;
};

class GfxManager {
  bool showDock_;
  GfxViewListener *listener_;
  Team **teams_;
  int numTeams_;
  Ship **ships_;
  int numShips_;
  bool initialized_;

  public:
    GfxManager(bool showDock);
    ~GfxManager();
    void setListener(GfxViewListener *listener);
    void processMouseClick(int mouseX, int mouseY);
    void initBbGfx(sf::RenderWindow *window, unsigned int viewHeight,
                   Stage *stage, Team **teams, int numTeams, Ship **ships,
                   int numShips, std::string resourcePath);
    void drawGame(sf::RenderWindow *window, Stage *stage, Ship **ships,
                  int numShips, int time, GfxEventHandler *gfxHandler,
                  bool gameOver);
    void updateView(sf::RenderWindow *window, unsigned int viewWidth,
                    unsigned int viewHeight);
    void destroyBbGfx();
  private:
    void drawDock(sf::RenderWindow *window, Stage *stage);
};

#endif
