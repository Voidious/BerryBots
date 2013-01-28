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

#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <platformstl/filesystem/readdir_sequence.hpp>
#include "ResourcePath.hpp"

#include "bbutil.h"
#include "bblua.h"
#include "stage.h"
#include "bbengine.h"
#include "gfxeventhandler.h"
#include "gfxmanager.h"
#include "filemanager.h"
#include "newmatch.h"
#include "outputconsole.h"
#include "printhandler.h"
#include "guiprinthandler.h"
#include "guimanager.h"
#include "basedir.h"
#include "bbwx.h"

extern BerryBotsEngine *engine;
extern Stage *stage;
extern PrintHandler *printHandler;

GuiManager::GuiManager() {
  window_ = new sf::RenderWindow(sf::VideoMode(800, 600), "BerryBots",
                                 sf::Style::Default,
                                 sf::ContextSettings(0, 0, 16, 2, 0));
  window_->setVisible(false);
  newMatchDialog_ = new NewMatchDialog();
  packageShipDialog_ = new PackageShipDialog();
  packageStageDialog_ = new PackageStageDialog();
  consoleId_ = 1000;
  stageConsole_ = 0;
  teamConsoles_ = 0;
  packagingConsole_ = new OutputConsole(this->nextConsoleId(),
                                        "Packaging Details");
  packagingConsole_->SetPosition(wxPoint(150, 100));
  numTeams_ = 0;
  gfxManager_ = new GfxManager(true);
  viewListener_ = new ViewListener(this);
  gfxManager_->setListener(viewListener_);
  fileManager_ = new FileManager();
  shipPackager_ = 0;
  stagePackager_ = 0;
  packageStageReporter_ = new PackageStageReporter(packagingConsole_);
  fileManager_->setListener(packageStageReporter_);
  newMatchDialog_->Show();
  newMatchDialog_->wxWindow::SetFocus();
  stageBaseDir_ = 0;
  botsBaseDir_ = 0;
  paused_ = false;
  matchId_ = 1;
  engine = 0;
}

GuiManager::~GuiManager() {
  deleteMatchConsoles();
  delete newMatchDialog_;
  delete packageShipDialog_;
  delete packageStageDialog_;
  delete packagingConsole_;
  if (stageBaseDir_ != 0) {
    delete stageBaseDir_;
  }
  if (botsBaseDir_ != 0) {
    delete botsBaseDir_;
  }
  if (engine != 0) {
    delete engine;
  }
  delete window_;
  delete gfxManager_;
  delete viewListener_;
  delete fileManager_;
  if (shipPackager_ != 0) {
    delete shipPackager_;
  }
  if (stagePackager_ != 0) {
    delete stagePackager_;
  }
  delete packageStageReporter_;
}

void GuiManager::loadStages(const char *baseDir) {
  if (stageBaseDir_ != 0) {
    delete stageBaseDir_;
  }
  stageBaseDir_ = new char[strlen(baseDir) + 1];
  strcpy(stageBaseDir_, baseDir);

  platformstl::readdir_sequence dir(baseDir,
                                    platformstl::readdir_sequence::files);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    if (isValidStageFile(baseDir, filename)) {
      newMatchDialog_->addStage(filename);
      if (fileManager_->isLuaFilename(filename)) {
        packageStageDialog_->addStage(filename);
      }
    }
  }
}

bool GuiManager::isValidStageFile(const char *baseDir, char *stageFilename) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       stages? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(stageFilename)
      || fileManager_->isZipFilename(stageFilename)) {
    int stagePathLen =
        (int) (strlen(baseDir) + strlen(BB_DIRSEP) + strlen(stageFilename));
    char *stagePath = new char[stagePathLen + 1];
    sprintf(stagePath, "%s%s%s", baseDir, BB_DIRSEP, stageFilename);
    char *cacheDir = getCacheDirCopy();
    char *stageDir = 0;
    char *stageFilename = 0;
    char *stageCwd = 0;
    try {
      fileManager_->loadStageFile(
          stagePath, &stageDir, &stageFilename, &stageCwd, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete stagePath;
      delete cacheDir;
      if (stageDir != 0) {
        delete stageDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      if (stageCwd != 0) {
        delete stageCwd;
      }
      return false;
    }
    delete stagePath;
    delete cacheDir;
    lua_State *stageState;
    initStageState(&stageState, stageCwd, stageFilename);
    
    if (luaL_loadfile(stageState, stageFilename)
        || lua_pcall(stageState, 0, 0, 0)) {
      lua_close(stageState);
      delete stageDir;
      delete stageFilename;
      delete stageCwd;
      return false;
    }
    
    lua_getglobal(stageState, "configure");
    if (lua_isnil(stageState, -1)) {
      lua_close(stageState);
      delete stageDir;
      delete stageFilename;
      delete stageCwd;
      return false;
    }

    lua_close(stageState);
    delete stageDir;
    delete stageFilename;
    delete stageCwd;
    return true;
  }
  return false;
}

void GuiManager::loadBots(const char *baseDir) {
  if (botsBaseDir_ != 0) {
    delete botsBaseDir_;
  }
  botsBaseDir_ = new char[strlen(baseDir) + 1];
  strcpy(botsBaseDir_, baseDir);

  platformstl::readdir_sequence dir(baseDir,
                                    platformstl::readdir_sequence::files);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    if (isValidBotFile(baseDir, filename)) {
      newMatchDialog_->addBot(filename);
      if (fileManager_->isLuaFilename(filename)) {
        packageShipDialog_->addBot(filename);
      }
    }
  }
}

bool GuiManager::isValidBotFile(const char *baseDir, char *botFilename) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       ships? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(botFilename)
      || fileManager_->isZipFilename(botFilename)) {
    int botPathLen =
        (int) (strlen(baseDir) + strlen(BB_DIRSEP) + strlen(botFilename));
    char *botPath = new char[botPathLen + 1];
    sprintf(botPath, "%s%s%s", baseDir, BB_DIRSEP, botFilename);
    char *cacheDir = getCacheDirCopy();
    char *botDir = 0;
    char *botFilename = 0;
    char *botCwd = 0;
    try {
      fileManager_->loadBotFile(
          botPath, &botDir, &botFilename, &botCwd, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete botPath;
      delete cacheDir;
      if (botDir != 0) {
        delete botDir;
      }
      if (botFilename != 0) {
        delete botFilename;
      }
      if (botCwd != 0) {
        delete botCwd;
      }
      return false;
    }
    delete botPath;
    delete cacheDir;
    lua_State *shipState;
    initShipState(&shipState, botCwd, botFilename);
    
    if (luaL_loadfile(shipState, botFilename)
        || lua_pcall(shipState, 0, 0, 0)) {
      lua_close(shipState);
      delete botDir;
      delete botFilename;
      delete botCwd;
      return false;
    }

    lua_getglobal(shipState, "configure");
    lua_getglobal(shipState, "init");
    if (lua_isnil(shipState, -1) || !lua_isnil(shipState, -2)) {
      lua_close(shipState);
      delete botDir;
      delete botFilename;
      delete botCwd;
      return false;
    }
    
    lua_close(shipState);
    delete botDir;
    delete botFilename;
    delete botCwd;
    return true;
  }
  return false;
}

void GuiManager::linkListeners() {
  newMatchDialog_->setListener(
      new MatchRunner(this, stageBaseDir_, botsBaseDir_));
  shipPackager_ = new ShipPackager(this, fileManager_, packagingConsole_,
                                   botsBaseDir_);
  stagePackager_ = new StagePackager(this, fileManager_, packagingConsole_,
                                     stageBaseDir_, botsBaseDir_);
  packageShipDialog_->setListener(shipPackager_);
  packageStageDialog_->setListener(stagePackager_);
}

void GuiManager::runMatch(char *stageName, char **teamNames, int numTeams) {
  unsigned int thisMatchId = matchId_;
  paused_ = false;
  newMatchDialog_->Hide();
  packageStageDialog_->Hide();
  packageShipDialog_->Hide();

  srand((unsigned int) time(NULL));
  engine = new BerryBotsEngine();
  stage = engine->getStage();
  char *cacheDir = getCacheDirCopy();
  try {
    engine->initStage(stageName, cacheDir);
    engine->initShips(teamNames, numTeams, cacheDir);
  } catch (EngineInitException *e) {
    // TODO: display error message in GUI
    delete engine;
    delete cacheDir;
    return;
  }
  delete cacheDir;
  GfxEventHandler *gfxHandler = new GfxEventHandler();
  stage->addEventHandler((EventHandler*) gfxHandler);
  
  unsigned int viewWidth = stage->getWidth() + (STAGE_MARGIN * 2);
  unsigned int viewHeight = stage->getHeight() + (STAGE_MARGIN * 2);
  unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
  unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth - DOCK_SIZE) / viewWidth,
                             ((double) screenHeight) / viewHeight));
  unsigned int targetWidth = floor(windowScale * viewWidth) + DOCK_SIZE;
  unsigned int targetHeight = floor(windowScale * viewHeight);
  window_->setSize(sf::Vector2u(targetWidth, targetHeight));
  gfxManager_->updateView(window_, viewWidth, viewHeight);

  // TODO: If/when SFML getPosition() works, adjust the window position to
  //       keep the whole window on the screen (if necessary). Might be worth
  //       platform-specific implementations using getSystemHandle() if that
  //       doesn't happen in a reasonable timeframe.
  //       We could just set it to (0, 0) or centered on screen every time, but
  //       that seems potentially super annoying to a user - less annoying than
  //       having to move the window occasionally if you switch to a bigger
  //       stage that goes off-screen.

  gfxManager_->initBbGfx(window_, viewHeight, stage, engine->getTeams(),
                         engine->getNumTeams(), engine->getShips(),
                         engine->getNumShips(), resourcePath());
  window_->setVisible(true);
  window_->clear();
  gfxManager_->drawGame(window_, stage, engine->getShips(),
                        engine->getNumShips(), engine->getGameTime(),
                        gfxHandler, false);
  window_->display();

  stageConsole_ = new OutputConsole(this->nextConsoleId(), stage->getName());
  stageConsole_->Hide();
  numTeams_ = numTeams;
  teamConsoles_ = new OutputConsole*[numTeams_];
  for (int x = 0; x < numTeams_; x++) {
    OutputConsole *teamConsole =
        new OutputConsole(this->nextConsoleId(), engine->getTeam(x)->name);
    teamConsole->Hide();
    teamConsoles_[x] = teamConsole;
  }
  printHandler = new GuiPrintHandler(stageConsole_, teamConsoles_, numTeams_);

  time_t realTime1;
  time_t realTime2;
  time(&realTime1);
  int realSeconds = 0;

  while (thisMatchId == matchId_ && window_->isOpen()) {
    if (!paused_ && !engine->isGameOver()) {
      engine->processTick();
    }

    sf::Event event;
    bool resized = false;
    while (window_->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window_->close();
      }
      if (event.type == sf::Event::KeyPressed
          && event.key.code == sf::Keyboard::Escape) {
        window_->close();
      }
      if (event.type == sf::Event::Resized && !resized) {
        resized = true;
        gfxManager_->updateView(window_, viewWidth, viewHeight);
      }
      if (event.type == sf::Event::MouseButtonPressed) {
        gfxManager_->processMouseClick(event.mouseButton.x,
                                       event.mouseButton.y);
      }
    }

    // TODO: Leaking ~2 MB per bot per match on my MacBook Pro (10.8, Cocoa).
    //       SFML folks seem to think it's in the video drivers.
    //       http://en.sfml-dev.org/forums/index.php?topic=8609.0
    //       Really need to investigate more and/or find a work-around.

    if (thisMatchId == matchId_) {
      window_->clear();
      gfxManager_->drawGame(window_, stage, engine->getShips(),
          engine->getNumShips(), engine->getGameTime(), gfxHandler,
          engine->isGameOver());
      window_->display();
      
      time(&realTime2);
      if (realTime2 - realTime1 > 0) {
        realSeconds++;
        // TODO: Display TPS in GUI.
      }
      realTime1 = realTime2;
    }
  }

  // TODO: Display winner / CPU usage in GUI

  gfxManager_->destroyBbGfx();
  delete printHandler;
  delete engine;
  delete gfxHandler;
  deleteMatchConsoles();
}

void GuiManager::showNewMatchDialog() {
  paused_ = true;
  newMatchDialog_->Show();
  newMatchDialog_->Raise();
}

void GuiManager::showPackageShipDialog() {
  paused_ = true;
  packageShipDialog_->Show();
  packageShipDialog_->Raise();
}

void GuiManager::showPackageStageDialog() {
  paused_ = true;
  packageStageDialog_->Show();
  packageStageDialog_->Raise();
}

void GuiManager::resumeMatch() {
  paused_ = false;
}

void GuiManager::cancelCurrentMatch() {
  matchId_++;
}

void GuiManager::showStageConsole() {
  stageConsole_->Show();
}

void GuiManager::showTeamConsole(int teamIndex) {
  teamConsoles_[teamIndex]->Show();
}

unsigned int GuiManager::nextConsoleId() {
  unsigned int nextId = consoleId_;
  consoleId_ += 100;
  return nextId;
}

void GuiManager::deleteMatchConsoles() {
  if (stageConsole_ != 0) {
    stageConsole_->Hide();
    delete stageConsole_;
    stageConsole_ = 0;
  }
  if (teamConsoles_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      teamConsoles_[x]->Hide();
      delete teamConsoles_[x];
    }
    delete teamConsoles_;
    teamConsoles_ = 0;
  }
}

void GuiManager::hideNewMatchDialog() {
  newMatchDialog_->Hide();
}

void GuiManager::hidePackageShipDialog() {
  packageShipDialog_->Hide();
}

void GuiManager::hidePackageStageDialog() {
  packageStageDialog_->Hide();
}

wxMenuBar* GuiManager::getNewMenuBar() {
  wxMenu *fileMenu = new wxMenu();
  fileMenu->Insert(0, NEW_MATCH_MENU_ID, "New Match...", 0);
  fileMenu->Insert(1, PACKAGE_SHIP_MENU_ID, "Package Ship...", 0);
  fileMenu->Insert(2, PACKAGE_STAGE_MENU_ID, "Package Stage...", 0);
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Insert(0, fileMenu, "File");
  return menuBar;
}

void GuiManager::quit() {
  matchId_ = 0;
}

MatchRunner::MatchRunner(GuiManager *guiManager, char *stageDir,
                         char *botsDir) {
  guiManager_ = guiManager;
  stageDir_ = new char[strlen(stageDir) + 1];
  strcpy(stageDir_, stageDir);
  botsDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsDir_, botsDir);
  matchRunning_ = false;
  nextStageName_ = 0;
  nextTeamNames_ = 0;
  nextNumTeams_ = 0;
  matchQueueMutex_ = new wxMutex();
}

MatchRunner::~MatchRunner() {
  delete stageDir_;
  delete botsDir_;
  delete matchQueueMutex_;
}

wxMenuBar* MatchRunner::getNewMenuBar() {
  return guiManager_->getNewMenuBar();
}

void MatchRunner::startMatch(const char *stageName, char **teamNames,
                             int numTeams) {
  queueNextMatch(stageName, teamNames, numTeams);
  if (matchRunning_) {
    guiManager_->cancelCurrentMatch();
  } else {
    matchRunning_ = true;
    while (nextStageName_ != 0) {
      matchQueueMutex_->Lock();

      int thisNumTeams = nextNumTeams_;
      unsigned long stagePathLen =
          strlen(stageDir_) + strlen(BB_DIRSEP) + strlen(nextStageName_);
      char *stagePath = new char[stagePathLen + 1];
      sprintf(stagePath, "%s%s%s", stageDir_, BB_DIRSEP, nextStageName_);
      char **teamPaths = new char*[thisNumTeams];
      for (int x = 0; x < thisNumTeams; x++) {
        unsigned long teamPathLen = strlen(botsDir_) + strlen(BB_DIRSEP)
            + strlen(nextTeamNames_[x]);
        char *teamPath = new char[teamPathLen + 1];
        sprintf(teamPath, "%s%s%s", botsDir_, BB_DIRSEP, nextTeamNames_[x]);
        teamPaths[x] = teamPath;
      }

      clearNextMatch();
      matchQueueMutex_->Unlock();

      guiManager_->runMatch(stagePath, teamPaths, thisNumTeams);

      delete stagePath;
      for (int x = 0; x < thisNumTeams; x++) {
        delete teamPaths[x];
      }
      delete teamPaths;

    }
    matchRunning_ = false;
  }
}

void MatchRunner::queueNextMatch(const char *stageName, char **teamNames,
                                 int numTeams) {
  matchQueueMutex_->Lock();
  
  nextStageName_ = new char[strlen(stageName) + 1];
  strcpy(nextStageName_, stageName);
  nextTeamNames_ = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    nextTeamNames_[x] = new char[strlen(teamNames[x]) + 1];
    strcpy(nextTeamNames_[x], teamNames[x]);
  }
  nextNumTeams_ = numTeams;

  matchQueueMutex_->Unlock();
}

void MatchRunner::clearNextMatch() {
  if (nextStageName_ != 0) {
    delete nextStageName_;
    nextStageName_ = 0;
  }
  if (nextTeamNames_ != 0) {
    for (int x = 0; x < nextNumTeams_; x++) {
      delete nextTeamNames_[x];
    }
    delete nextTeamNames_;
    nextTeamNames_ = 0;
  }
}

void MatchRunner::cancel() {
  guiManager_->hideNewMatchDialog();
  guiManager_->resumeMatch();
}

ShipPackager::ShipPackager(GuiManager *guiManager, FileManager *fileManager,
                           OutputConsole *packagingConsole, char *botsDir) {
  packagingConsole_ = packagingConsole;
  guiManager_ = guiManager;
  fileManager_ = fileManager;
  botsDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsDir_, botsDir);
}

ShipPackager::~ShipPackager() {
  delete botsDir_;
}

wxMenuBar* ShipPackager::getNewMenuBar() {
  return guiManager_->getNewMenuBar();
}

void ShipPackager::package(const char *botName, const char *version,
                           bool nosrc) {
  char *botsPath = new char[strlen(botsDir_) + strlen(botName) + 2];
  sprintf(botsPath, "%s%s%s", botsDir_, BB_DIRSEP, botName);
  char *cacheDir = getCacheDirCopy();
  char *tmpDir = getTmpDirCopy();
  try {
    fileManager_->packageBot(botsPath, version, cacheDir, tmpDir, nosrc);
  } catch (FileNotFoundException *e) {
    delete botsPath;
    delete cacheDir;
    delete tmpDir;
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging ship failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
  }
  delete botsPath;
  delete cacheDir;
  delete tmpDir;
}

void ShipPackager::cancel() {
  guiManager_->hidePackageShipDialog();
  guiManager_->resumeMatch();
}

StagePackager::StagePackager(GuiManager *guiManager, FileManager *fileManager,
    OutputConsole *packagingConsole, char *stageDir, char *botsDir) {
  packagingConsole_ = packagingConsole;
  guiManager_ = guiManager;
  fileManager_ = fileManager;
  stageDir_ = new char[strlen(stageDir) + 1];
  strcpy(stageDir_, stageDir);
  botsDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsDir_, botsDir);
}

StagePackager::~StagePackager() {
  delete stageDir_;
  delete botsDir_;
}

wxMenuBar* StagePackager::getNewMenuBar() {
  return guiManager_->getNewMenuBar();
}

void StagePackager::package(const char *stageName, const char *version,
                            bool nosrc) {
  char *stagePath = new char[strlen(stageDir_) + strlen(stageName) + 2];
  sprintf(stagePath, "%s%s%s", stageDir_, BB_DIRSEP, stageName);
  char *cacheDir = getCacheDirCopy();
  char *tmpDir = getTmpDirCopy();
  try {
    fileManager_->packageStage(stagePath, version, cacheDir, tmpDir, nosrc);
  } catch (FileNotFoundException *e) {
    delete stagePath;
    delete cacheDir;
    delete tmpDir;
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging stage failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
  }
  delete stagePath;
  delete cacheDir;
  delete tmpDir;
}

void StagePackager::cancel() {
  guiManager_->hidePackageStageDialog();
  guiManager_->resumeMatch();
}

PackageStageReporter::PackageStageReporter(OutputConsole *packagingConsole) {
  packagingConsole_ = packagingConsole;
}

void PackageStageReporter::packagingComplete(char **sourceFiles, int numFiles,
                                             const char *destinationFile) {
  packagingConsole_->clear();
  packagingConsole_->Show();
  packagingConsole_->println("The following source files were packaged:");
  for (int x = 0; x < numFiles; x++) {
    if (sourceFiles[x] != 0) {
      packagingConsole_->print("  ");
      packagingConsole_->println(sourceFiles[x]);
    }
  }
  packagingConsole_->println();
  packagingConsole_->print("Saved to: ");
  packagingConsole_->println(destinationFile);
}

ViewListener::ViewListener(GuiManager *guiManager) {
  guiManager_ = guiManager;
}

void ViewListener::onNewMatch() {
  guiManager_->showNewMatchDialog();
}

void ViewListener::onStageClick() {
  guiManager_->showStageConsole();
}

void ViewListener::onTeamClick(int teamIndex) {
  guiManager_->showTeamConsole(teamIndex);
}
