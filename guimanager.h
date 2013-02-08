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
#include "filemanager.h"
#include "newmatch.h"
#include "packagedialog.h"
#include "packageship.h"
#include "packagestage.h"
#include "outputconsole.h"

class GuiListener {
  public:
    virtual void onAllWindowsClosed() = 0;
    virtual ~GuiListener() {};
};

class GuiManager {
  GuiListener *listener_;
  NewMatchDialog *newMatchDialog_;
  PackageShipDialog *packageShipDialog_;
  PackageStageDialog *packageStageDialog_;
  sf::RenderWindow *window_;
  OutputConsole *stageConsole_;
  OutputConsole **teamConsoles_;
  OutputConsole *packagingConsole_;
  GfxManager *gfxManager_;
  GfxViewListener *viewListener_;
  Zipper *zipper_;
  FileManager *fileManager_;
  PackageDialogListener *shipPackager_;
  PackageDialogListener *stagePackager_;
  PackagingListener *packageReporter_;
  BerryBotsEngine *engine_;
  char *stageBaseDir_;
  char *botsBaseDir_;
  unsigned int consoleId_;
  char *currentStagePath_;
  char **currentTeamPaths_;
  int currentNumTeams_;
  unsigned int viewWidth_;
  unsigned int viewHeight_;
  GfxEventHandler *gfxHandler_;
  // Interrupted means the user brought up a modal dialog (eg, "New Match")...
  // The run loop exits but everything stays initialized. Match may be resumed
  // if user closes the New Match window, or clobbered to start a new match.
  bool interrupted_;
  // Paused means the game engine is not executing the match, but the SFML
  // window is still being drawn and listening for events. (Either the user
  // paused the game or it ended and they haven't done anything yet.)
  bool paused_;
  bool restarting_;
  bool quitting_;
  
  public:
    GuiManager(GuiListener *listener, char *stageDir, char *botsDir);
    ~GuiManager();
    void loadStages();
    bool isValidStageFile(const char *baseDir, char *stageFilename);
    void loadBots();
    bool isValidBotFile(const char *baseDir, char *stageFilename);
    void runNewMatch(char *stagePath, char **teamPaths, int numTeams);
    void resumeMatch();
    void processMainWindowEvents();
    void showNewMatchDialog();
    void showPackageShipDialog();
    void showPackageStageDialog();
    void showStageConsole();
    void showTeamConsole(int teamIndex);
    void hideNewMatchDialog();
    void hidePackageShipDialog();
    void hidePackageStageDialog();
    wxMenuBar* getNewMenuBar();
    void togglePause();
    void restartMatch();
    void quit();
    char* getStageDirCopy();
    char* getBotsDirCopy();
    char* getCacheDirCopy();
    char* getTmpDirCopy();
  private:
    sf::RenderWindow* initMainWindow(unsigned int width, unsigned int height);
    sf::RenderWindow* getMainWindow();
    void runCurrentMatch();
    unsigned int nextConsoleId();
    void deleteMatchConsoles();
    void saveCurrentMatchSettings(
        char *stagePath, char **teamPaths, int numTeams);
    void deleteCurrentMatchSettings();
};

class MatchRunner : public NewMatchListener {
  GuiManager *guiManager_;
  char *stageDir_;
  char *botsDir_;

  public:
    MatchRunner(GuiManager *guiManager, char *stageDir, char *botsDir);
    ~MatchRunner();
    virtual wxMenuBar* getNewMenuBar();
    virtual void startMatch(const char *stageName, char **teamNames,
                            int numTeams);
    virtual void refreshFiles();
    virtual void cancel();
};

class ShipPackager : public PackageDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *botsDir_;

  public:
    ShipPackager(GuiManager *guiManager, FileManager *fileManager,
                 OutputConsole *packagingConsole, char *botsDir);
    ~ShipPackager();
    virtual wxMenuBar* getNewMenuBar();
    virtual void package(const char *botName, const char *version, bool nosrc);
    virtual void cancel();
};

class StagePackager : public PackageDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *stageDir_;
  char *botsDir_;

  public:
    StagePackager(GuiManager *guiManager, FileManager *fileManager,
        OutputConsole *packagingConsole, char *stageDir, char *botsDir);
    ~StagePackager();
    virtual wxMenuBar* getNewMenuBar();
    virtual void package(const char *stageName, const char *version,
                         bool nosrc);
    virtual void cancel();
};

class PackageReporter : public PackagingListener {
  OutputConsole *packagingConsole_;

  public:
    PackageReporter(OutputConsole *packagingConsole);
    virtual void packagingComplete(char **sourceFiles, int numFiles, bool nosrc,
                                   const char *destinationFile);
};

class ViewListener : public GfxViewListener {
  GuiManager *guiManager_;
  public:
    ViewListener(GuiManager *guiManager);
    virtual void onNewMatch();
    virtual void onPackageShip();
    virtual void onPackageStage();
    virtual void onStageClick();
    virtual void onTeamClick(int teamIndex);
    virtual void onPauseUnpause();
    virtual void onRestart();
};

#endif
