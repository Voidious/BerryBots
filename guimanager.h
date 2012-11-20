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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <SFML/Graphics.hpp>
#include "gfxmanager.h"
#include "newmatch.h"
#include "outputconsole.h"

class GuiManager {
  NewMatchDialog *newMatchDialog_;
  sf::RenderWindow *window_;
  OutputConsole *stageConsole_;
  OutputConsole **teamConsoles_;
  int numTeams_;
  GfxManager *gfxManager_;
  char *stageBaseDir_;
  char *botsBaseDir_;
  bool paused_;
  unsigned int consoleId_;
  unsigned int matchId_;

  public:
    GuiManager();
    ~GuiManager();
    void loadStages(const char *baseDir);
    bool isValidStageFile(const char *baseDir, char *stageFilename);
    void loadBots(const char *baseDir);
    bool isValidBotFile(const char *baseDir, char *stageFilename);
    void linkListener();
    void runMatch(char *stageName, char **teamNames, int numTeams);
    void showNewMatchDialog();
    void resumeMatch();
    void showStageConsole();
    void showTeamConsole(int teamIndex);
  private:
    unsigned int nextConsoleId();
    void clearConsoles();
};

class MatchStarter : public NewMatchListener {
  GuiManager *guiManager_;
  char *stageDir_;
  char *botsDir_;
  public:
    MatchStarter(GuiManager *guiManager, char *stageDir, char *botsDir);
    ~MatchStarter();
    virtual void startMatch(const char *stageName, const char **teamNames,
                            int numTeams);
    virtual void cancel();
};

class ViewListener : public GfxViewListener {
  GuiManager *guiManager_;
  public:
    ViewListener(GuiManager *guiManager);
    virtual void onNewMatch();
    virtual void onStageClick();
    virtual void onTeamClick(int teamIndex);
};

#endif
