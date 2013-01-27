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
#include <wx/thread.h>
#include "gfxmanager.h"
#include "filemanager.h"
#include "newmatch.h"
#include "packageship.h"
#include "packagestage.h"
#include "outputconsole.h"

class GuiManager {
  NewMatchDialog *newMatchDialog_;
  PackageShipDialog *packageShipDialog_;
  PackageStageDialog *packageStageDialog_;
  sf::RenderWindow *window_;
  OutputConsole *stageConsole_;
  OutputConsole **teamConsoles_;
  OutputConsole *packagingConsole_;
  int numTeams_;
  GfxManager *gfxManager_;
  GfxViewListener *viewListener_;
  FileManager *fileManager_;
  PackageShipDialogListener *shipPackager_;
  PackageStageDialogListener *stagePackager_;
  PackagingListener *packageStageReporter_;
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
    void linkListeners();
    void runMatch(char *stageName, char **teamNames, int numTeams);
    void showNewMatchDialog();
    void showPackageShipDialog();
    void showPackageStageDialog();
    void resumeMatch();
    void cancelCurrentMatch();
    void showStageConsole();
    void showTeamConsole(int teamIndex);
    void hideNewMatchDialog();
    void hidePackageShipDialog();
    void hidePackageStageDialog();
    void quit();
  private:
    unsigned int nextConsoleId();
    void deleteMatchConsoles();
};

class MatchRunner : public NewMatchListener {
  GuiManager *guiManager_;
  char *stageDir_;
  char *botsDir_;
  bool matchRunning_;
  char *nextStageName_;
  char **nextTeamNames_;
  int nextNumTeams_;
  wxMutex *matchQueueMutex_;

  public:
    MatchRunner(GuiManager *guiManager, char *stageDir, char *botsDir);
    ~MatchRunner();
    virtual void startMatch(const char *stageName, char **teamNames,
                            int numTeams);
    virtual void cancel();
  private:
    void queueNextMatch(const char *stageName, char **teamNames, int numTeams);
    void clearNextMatch();
};

class ShipPackager : public PackageShipDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *botsDir_;
  
  public:
    ShipPackager(GuiManager *guiManager, FileManager *fileManager,
                 OutputConsole *packagingConsole, char *botsDir);
    ~ShipPackager();
    virtual void package(const char *botName, const char *version, bool nosrc);
    virtual void cancel();
};

class StagePackager : public PackageStageDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *stageDir_;
  char *botsDir_;

  public:
    StagePackager(GuiManager *guiManager, FileManager *fileManager,
        OutputConsole *packagingConsole, char *stageDir, char *botsDir);
    ~StagePackager();
    virtual void package(const char *stageName, const char *version,
                         bool nosrc);
    virtual void cancel();
};

class PackageStageReporter : public PackagingListener {
  OutputConsole *packagingConsole_;

  public:
    PackageStageReporter(OutputConsole *packagingConsole);
    virtual void packagingComplete(char **sourceFiles, int numFiles,
                                   const char *destinationFile);
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
