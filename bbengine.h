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

#ifndef BBENGINE_H
#define BBENGINE_H

#include "bbconst.h"
#include "bbutil.h"
#include "stage.h"
#include "sensorhandler.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

class BerryBotsEngine {
  Stage *stage_;
  lua_State *stageState_;
  char *stageDir_;
  char *stageFilename_;
  World *stageWorld_;

  Team **teams_;
  Ship **ships_;
  Ship **stageShips_;
  Ship **oldShips_;
  ShipProperties **shipProperties_;
  int numTeams_;
  int teamSize_;
  int numShips_;
  World **worlds_;
  bool** teamVision_;

  int gameTime_;
  SensorHandler *sensorHandler_;
  bool stageRun_;
  bool configureComplete_;
  bool initComplete_;
  bool battleMode_;
  bool roundOver_;
  bool gameOver_;
  int cpuTimeSlot_;
  char winnerName_[MAX_NAME_LENGTH + 1];

  public:
    BerryBotsEngine();
    ~BerryBotsEngine();

    bool isConfigureComplete();
    bool isInitComplete();
    void setBattleMode(bool battleMode);
    bool isBattleMode();
    void nextRound();
    void setRoundOver(bool roundOver);
    bool isRoundOver();
    void setGameOver(bool gameOver);
    bool isGameOver();
    void setWinnerName(const char* winnerName);
    char* getWinnerName();

    void initStage(char *stagePath, const char *cacheDir);
    void initShips(char **teamPaths, int numTeams, const char *cacheDir);
    void processTick();
    void processRoundOver();
    void processGameOver();

    Stage* getStage();
    Team **getTeams();
    Team *getTeam(int teamIndex);
    Team *getTeam(lua_State *L);
    int getNumTeams();
    Ship** getShips();
    int getNumShips();
    int getGameTime();
    void setTeamSize(int teamSize);
    int getTeamSize();

    bool touchedZone(Ship *ship, const char *zoneTag);
    bool touchedAnyZone(Ship *ship);
    void destroyShip(Ship *ship);
  private:
    void initShipRound(Ship *ship);
    void updateTeamShipsAlive();
    void processStageRun();
    void uniqueShipNames(Ship** ships, int numShips);
    void uniqueTeamNames(Team** teams, int numTeams);
    void copyShips(Ship **srcShips, Ship **destShips, int numShips);
};

#endif
