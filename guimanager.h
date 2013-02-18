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
#include "menubarmaker.h"
#include "guiprinthandler.h"

#define ITEM_STAGE  1
#define ITEM_BOT    2

class GuiListener {
  public:
    virtual void onAllWindowsClosed() = 0;
    virtual ~GuiListener() {};
};

class PrintStateListener : public NewTeamStateListener {
  GuiPrintHandler *guiPrintHandler_;
  
  public:
    PrintStateListener(GuiPrintHandler *guiPrintHandler);
    virtual void newTeamState(lua_State *teamState, const char *filename);
    OutputConsole** getTeams();
};

class GuiManager {
  GuiListener *listener_;
  NewMatchDialog *newMatchDialog_;
  PackageShipDialog *packageShipDialog_;
  PackageStageDialog *packageStageDialog_;
  sf::RenderWindow *window_;
  sf::RenderWindow *previewWindow_;
  OutputConsole *stageConsole_;
  OutputConsole **teamConsoles_;
  OutputConsole *packagingConsole_;
  OutputConsole *errorConsole_;
  MenuBarMaker *menuBarMaker_;
  GfxManager *gfxManager_;
  GfxManager *previewGfxManager_;
  GfxViewListener *viewListener_;
  sf::Image windowIcon_;
  Zipper *zipper_;
  FileManager *fileManager_;
  NewMatchListener *newMatchListener_;
  PackageDialogListener *shipPackager_;
  PackageDialogListener *stagePackager_;
  PackagingListener *packageReporter_;
  PrintStateListener *printStateListener_;
  BerryBotsEngine *engine_;
  char *stageBaseDir_;
  char *botsBaseDir_;
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
  bool previewing_;
  bool closingPreview_;
  double tpsFactor_;
  double nextDrawTime_;
  
  public:
    GuiManager(GuiListener *listener);
    ~GuiManager();
    void setBaseDirs(const char *stagesBaseDir, const char *botsBaseDir);
    void reloadBaseDirs();
    void loadStages();
    bool isValidStageFile(const char *srcFilename, BerryBotsEngine *engine);
    void loadBots();
    bool isValidBotFile(const char *srcFilename, BerryBotsEngine *engine);
    void runNewMatch(const char *stageName, char **teamNames, int numTeams);
    void processMainWindowEvents(sf::RenderWindow *window,
        GfxManager *gfxManager, int viewWidth, int viewHeight);
    void processPreviewWindowEvents(sf::RenderWindow *window,
        GfxManager *gfxManager, int viewWidth, int viewHeight);
    void showNewMatchDialog();
    void showPackageShipDialog();
    void showPackageStageDialog();
    void showStageConsole();
    void showTeamConsole(int teamIndex);
    void showErrorConsole();
    void showStagePreview(const char *stageName);
    void closeStagePreview();
    void hideNewMatchDialog();
    void hidePackageShipDialog();
    void hidePackageStageDialog();
    void hidePackagingConsole();
    void hideErrorConsole();
    void dialogClosed();
    void dialogEscaped();
    void newMatchInitialFocus();
    void packageShipInitialFocus();
    void packageStageInitialFocus();
    void togglePause();
    void restartMatch();
    void setTpsFactor(double tpsFactor);
    void quit();
    char* getStageDirCopy();
    char* getBotsDirCopy();
    char* getCacheDirCopy();
    char* getTmpDirCopy();
  private:
    sf::RenderWindow* initMainWindow(unsigned int width, unsigned int height);
    sf::RenderWindow* getMainWindow();
    void runCurrentMatch();
    void resumeMatch();
    void deleteMatchConsoles();
    void saveCurrentMatchSettings(
        const char *stageName, char **teamNames, int numTeams);
    void deleteCurrentMatchSettings();
    void loadStagesFromDir(const char *loadDir);
    void loadBotsFromDir(const char *loadDir);
    void loadItemsFromDir(const char *baseDir, const char *loadDir,
        int itemType, PackageDialog *packageDialog, BerryBotsEngine *engine);
    void logErrorMessage(lua_State *L, const char *formatString);
};

class MatchRunner : public NewMatchListener {
  GuiManager *guiManager_;
  char *stageDir_;
  char *botsDir_;

  public:
    MatchRunner(GuiManager *guiManager, char *stageDir, char *botsDir);
    ~MatchRunner();
    virtual void startMatch(const char *stageName, char **teamNames,
                            int numTeams);
    virtual void previewStage(const char *stagePath);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
    virtual void onActive();
    virtual void reloadBaseDirs();
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
    virtual void package(const char *botName, const char *version,
                         bool obfuscate);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
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
    virtual void package(const char *stageName, const char *version,
                         bool obfuscate);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
};

class PackageReporter : public PackagingListener {
  OutputConsole *packagingConsole_;

  public:
    PackageReporter(OutputConsole *packagingConsole);
    virtual void packagingComplete(char **sourceFiles, int numFiles,
                                   bool obfuscate, const char *destinationFile);
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
    virtual void onTpsChange(double tpsFactor);
};

#endif
