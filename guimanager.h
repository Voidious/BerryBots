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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <SFML/Graphics.hpp>
#include "gfxmanager.h"
#include "filemanager.h"
#include "guigamerunner.h"
#include "newmatch.h"
#include "packagedialog.h"
#include "packageship.h"
#include "packagestage.h"
#include "runnerdialog.h"
#include "outputconsole.h"
#include "menubarmaker.h"
#include "guiprinthandler.h"

#define ITEM_STAGE   1
#define ITEM_SHIP    2
#define ITEM_RUNNER  3

#define TOO_MANY_RECTANGLES  "== Warning: Tried to draw too many DebugGfx rectangles (max 4096)."
#define TOO_MANY_LINES       "== Warning: Tried to draw too many DebugGfx lines (max 4096)."
#define TOO_MANY_CIRCLES     "== Warning: Tried to draw too many DebugGfx circles (max 4096)."
#define TOO_MANY_TEXTS       "== Warning: Tried to draw too many DebugGfx texts (max 4096)."
#define TOO_MANY_MORE_INFO   " Some of your graphics are being ignored. See API docs for more info."

class GuiListener {
  public:
    virtual void onAllWindowsClosed() = 0;
    virtual ~GuiListener() {};
};

class PrintStateListener : public NewTeamStateListener {
  GuiPrintHandler *guiPrintHandler_;
  
  public:
    PrintStateListener(GuiPrintHandler *guiPrintHandler);
    virtual void newTeam(Team *team, const char *filename);
    OutputConsole** getTeams();
};

class ConsoleEventHandler;

class GuiManager {
  GuiListener *listener_;
  NewMatchDialog *newMatchDialog_;
  PackageShipDialog *packageShipDialog_;
  PackageStageDialog *packageStageDialog_;
  RunnerDialog *runnerDialog_;
  sf::RenderWindow *window_;
  sf::RenderWindow *previewWindow_;
  OutputConsole *stageConsole_;
  OutputConsole **teamConsoles_;
  OutputConsole *packagingConsole_;
  OutputConsole *errorConsole_;
  OutputConsole *runnerConsole_;
  OutputConsole *previewConsole_;
  ConsoleListener *previewConsoleListener_;
  MenuBarMaker *menuBarMaker_;
  GfxManager *gfxManager_;
  GfxManager *previewGfxManager_;
  GfxViewListener *viewListener_;
  sf::Image windowIcon_;
  Zipper *zipper_;
  FileManager *fileManager_;
  GuiGameRunner *gameRunner_;
  NewMatchListener *newMatchListener_;
  PackageDialogListener *shipPackager_;
  PackageDialogListener *stagePackager_;
  RunnerDialogListener *runnerLauncher_;
  PackagingListener *packageReporter_;
  PrintStateListener *printStateListener_;
  BerryBotsEngine *engine_;
  char *stagesBaseDir_;
  char *shipsBaseDir_;
  char *runnersBaseDir_;
  char *currentStagePath_;
  char **currentTeamPaths_;
  int currentNumTeams_;
  unsigned int viewWidth_;
  unsigned int viewHeight_;
  GfxEventHandler *gfxHandler_;
  ConsoleEventHandler *consoleHandler_;
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
  bool tooManyStageRectangles_;
  bool tooManyStageLines_;
  bool tooManyStageCircles_;
  bool tooManyStageTexts_;
  int numStages_;
  int numShips_;
  int numRunners_;
  
  public:
    GuiManager(GuiListener *listener);
    ~GuiManager();
    void setBaseDirs(const char *stagesBaseDir, const char *shipsBaseDir,
                     const char *runnersBaseDir);
    void reloadBaseDirs();
    void loadStages();
    bool isValidStageFile(const char *srcFilename, BerryBotsEngine *engine);
    void loadShips();
    bool isValidShipFile(const char *srcFilename, BerryBotsEngine *engine);
    void loadRunners();
    bool isValidRunnerFile(const char *srcFilename, BerryBotsEngine *engine);
    void startMatch(const char *stageName, char **teamNames, int numTeams);
    void runNewMatch(const char *stageName, char **teamNames, int numTeams);
    void processMainWindowEvents(sf::RenderWindow *window,
        GfxManager *gfxManager, int viewWidth, int viewHeight);
    void processPreviewWindowEvents(sf::RenderWindow *window,
        GfxManager *gfxManager, int viewWidth, int viewHeight);
    void launchGameRunner(const char *runnerName);
    void showNewMatchDialog();
    void showPackageShipDialog();
    void showPackageStageDialog();
    void showGameRunnerDialog();
    void showStageConsole();
    void showTeamConsole(int teamIndex);
    void showErrorConsole();
    void showStagePreview(const char *stageName);
    void closeStagePreview();
    void hideNewMatchDialog();
    void hidePackageShipDialog();
    void hidePackageStageDialog();
    void hideGameRunnerDialog();
    void hidePackagingConsole();
    void hideErrorConsole();
    void dialogClosed();
    void dialogEscaped();
    void newMatchInitialFocus();
    void packageShipInitialFocus();
    void packageStageInitialFocus();
    void gameRunnerInitialFocus();
    void printShipDestroyed(Ship *destroyedShip, Ship *destroyerShip, int time);
    void printTooManyUserGfxRectangles(Team *team);
    void printTooManyUserGfxLines(Team *team);
    void printTooManyUserGfxCircles(Team *team);
    void printTooManyUserGfxTexts(Team *team);
    void togglePause();
    void restartMatch();
    void setTpsFactor(double tpsFactor);
    void quit();
    void redrawMainWindow();
    char* getStagesDirCopy();
    char* getShipsDirCopy();
    char* getCacheDirCopy();
    char* getTmpDirCopy();
  private:
    sf::RenderWindow* initMainWindow(unsigned int width, unsigned int height);
    sf::RenderWindow* initPreviewWindow(unsigned int width,
                                        unsigned int height);
    sf::RenderWindow* getMainWindow();
    void runCurrentMatch();
    void drawFrame(sf::RenderWindow *window);
    void clearTeamErroredForActiveConsoles(BerryBotsEngine *engine);
    void resumeMatch();
    void showDialog(wxFrame *dialog);
    void deleteStageConsole();
    void saveCurrentMatchSettings(
        const char *stageName, char **teamNames, int numTeams);
    void deleteCurrentMatchSettings();
    void loadStagesFromDir(const char *loadDir);
    void loadShipsFromDir(const char *loadDir);
    int loadItemsFromDir(const char *baseDir, const char *loadDir,
        int itemType, void *itemDialog, BerryBotsEngine *engine);
    void logErrorMessage(lua_State *L, const char *formatString);
};

class MatchStarter : public NewMatchListener {
  GuiManager *guiManager_;
  char *stagesDir_;
  char *shipsDir_;

  public:
    MatchStarter(GuiManager *guiManager, char *stagesDir, char *shipsDir);
    ~MatchStarter();
    virtual void startMatch(const char *stageName, char **teamNames,
                            int numTeams);
    virtual void previewStage(const char *stagePath);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
    virtual void onActive();
    virtual void onUpdateUi();
    virtual void reloadBaseDirs();
};

class ShipPackager : public PackageDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *shipsDir_;

  public:
    ShipPackager(GuiManager *guiManager, FileManager *fileManager,
                 OutputConsole *packagingConsole, char *shipsDir);
    ~ShipPackager();
    virtual void package(const char *shipName, const char *version,
                         bool obfuscate);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
    virtual void onUpdateUi();
};

class StagePackager : public PackageDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *packagingConsole_;
  char *stagesDir_;
  char *shipsDir_;

  public:
    StagePackager(GuiManager *guiManager, FileManager *fileManager,
        OutputConsole *packagingConsole, char *stagesDir, char *shipsDir);
    ~StagePackager();
    virtual void package(const char *stageName, const char *version,
                         bool obfuscate);
    virtual void refreshFiles();
    virtual void onClose();
    virtual void onEscape();
    virtual void onUpdateUi();
};

class RunnerLauncher : public RunnerDialogListener {
  GuiManager *guiManager_;
  FileManager *fileManager_;
  OutputConsole *runnerConsole_;
  char *runnersDir_;
  
public:
  RunnerLauncher(GuiManager *guiManager, FileManager *fileManager,
                 OutputConsole *runnerConsole, char *runnersDir);
  ~RunnerLauncher();
  virtual void launch(const char *runnerName);
  virtual void refreshFiles();
  virtual void onClose();
  virtual void onEscape();
  virtual void onUpdateUi();
};

class PackageReporter : public PackagingListener {
  OutputConsole *packagingConsole_;

  public:
    PackageReporter(OutputConsole *packagingConsole);
    virtual void packagingComplete(char **sourceFiles, int numFiles,
                                   bool obfuscate, const char *destinationFile);
};

class ConsoleEventHandler : public EventHandler {
  GuiManager *guiManager_;

  public:
    ConsoleEventHandler(GuiManager *guiManager);
    virtual void handleShipDestroyed(Ship *destroyedShip, int time,
        Ship **destroyerShips, int numDestroyers);
    virtual void tooManyUserGfxRectangles(Team *team);
    virtual void tooManyUserGfxLines(Team *team);
    virtual void tooManyUserGfxCircles(Team *team);
    virtual void tooManyUserGfxTexts(Team *team);

    virtual void handleLaserHitShip(Ship *srcShip, Ship *targetShip, double dx,
        double dy, double laserX, double laserY, double laserHeading,
        int time) {};
    virtual void handleTorpedoExploded(double x, double y, int time) {};
    virtual void handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
        double dx, double dy, double hitAngle, double hitForce,
        double hitDamage, int time) {};
    virtual void handleShipHitShip(Ship *hittingShip, Ship *targetShip,
        double inAngle, double inForce, double outAngle, double outForce,
        int time) {};
    virtual void handleShipHitWall(
        Ship *hittingShip, double bounceAngle, double bounceForce, int time) {};
    virtual void handleShipFiredLaser(
        Ship *firingShip, double laserHeading, int time) {};
    virtual void handleShipFiredTorpedo(Ship *firingShip, double torpedoHeading,
        double torpedoDistance, int time) {};
};

class StageConsoleListener : public ConsoleListener {
  Stage *stage_;

  public:
    StageConsoleListener(Stage *stage);
    virtual void onActive() {};
    virtual void onClose() {};
    virtual void onCheck(bool checked);
    virtual ~StageConsoleListener() {};
};

class PreviewConsoleListener : public ConsoleListener {
  GuiManager *guiManager_;

  public:
    PreviewConsoleListener(GuiManager *guiManager);
    virtual void onActive() {};
    virtual void onClose();
    virtual void onCheck(bool checked) {};
    virtual ~PreviewConsoleListener() {};
  
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
