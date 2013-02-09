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
#include <exception>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <wx/wx.h>
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
#include "guizipper.h"

extern PrintHandler *printHandler;

GuiManager::GuiManager(GuiListener *listener, char *stageDir, char *botsDir) {
  listener_ = listener;
  stageBaseDir_ = new char[strlen(stageDir) + 1];
  strcpy(stageBaseDir_, stageDir);
  botsBaseDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsBaseDir_, botsDir);

  window_ = 0;
  consoleId_ = 1000;
  zipper_ = new GuiZipper();
  fileManager_ = new FileManager(zipper_);
  menuBarMaker_ = new MenuBarMaker();
  packagingConsole_ = new OutputConsole("Packaging Details", menuBarMaker_);
  packagingConsole_->SetPosition(wxPoint(150, 100));
  newMatchListener_ = new MatchRunner(this, stageBaseDir_, botsBaseDir_);
  newMatchDialog_ = new NewMatchDialog(newMatchListener_, menuBarMaker_);
  shipPackager_ = new ShipPackager(this, fileManager_, packagingConsole_,
                                   botsBaseDir_);
  packageShipDialog_ = new PackageShipDialog(shipPackager_, menuBarMaker_);
  stagePackager_ = new StagePackager(this, fileManager_, packagingConsole_,
                                     stageBaseDir_, botsBaseDir_);
  packageStageDialog_ = new PackageStageDialog(stagePackager_, menuBarMaker_);
  loadStages();
  loadBots();

  stageConsole_ = 0;
  teamConsoles_ = 0;
  gfxManager_ = new GfxManager(true);
  viewListener_ = new ViewListener(this);
  gfxManager_->setListener(viewListener_);
  packageReporter_ = new PackageReporter(packagingConsole_);
  fileManager_->setListener(packageReporter_);
  newMatchDialog_->Show();
  newMatchDialog_->SetFocus();
  engine_ = 0;
  currentStagePath_ = 0;
  currentTeamPaths_ = 0;
  currentNumTeams_ = 0;
  interrupted_ = false;
  paused_ = false;
  restarting_ = false;
  quitting_ = false;
}

GuiManager::~GuiManager() {
  deleteMatchConsoles();
  deleteCurrentMatchSettings();
  delete newMatchDialog_;
  delete packageShipDialog_;
  delete packageStageDialog_;
  delete packagingConsole_;
  delete menuBarMaker_;
  delete stageBaseDir_;
  delete botsBaseDir_;
  if (engine_ != 0) {
    delete engine_;
  }
  if (window_ != 0) {
    delete window_;
  }
  delete gfxManager_;
  delete viewListener_;
  delete zipper_;
  delete fileManager_;
  delete newMatchListener_;
  delete shipPackager_;
  delete stagePackager_;
  delete packageReporter_;
}

void GuiManager::loadStages() {
  newMatchDialog_->clearStages();
  packageStageDialog_->clearItems();
  // TODO: move file related stuff to FileManager
  platformstl::readdir_sequence dir(stageBaseDir_,
                                    platformstl::readdir_sequence::files);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    if (isValidStageFile(stageBaseDir_, filename)) {
      newMatchDialog_->addStage(filename);
      if (fileManager_->isLuaFilename(filename)) {
        packageStageDialog_->addItem(filename);
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

void GuiManager::loadBots() {
  newMatchDialog_->clearBots();
  packageShipDialog_->clearItems();
  // TODO: move file related stuff to FileManager
  platformstl::readdir_sequence dir(botsBaseDir_,
                                    platformstl::readdir_sequence::files);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    if (isValidBotFile(botsBaseDir_, filename)) {
      newMatchDialog_->addBot(filename);
      if (fileManager_->isLuaFilename(filename)) {
        packageShipDialog_->addItem(filename);
      }
    }
  }
  newMatchDialog_->removeStaleLoadedBots();
}

bool GuiManager::isValidBotFile(const char *baseDir, char *origFilename) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       ships? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(origFilename)
      || fileManager_->isZipFilename(origFilename)) {
    int botPathLen =
        (int) (strlen(baseDir) + strlen(BB_DIRSEP) + strlen(origFilename));
    char *botPath = new char[botPathLen + 1];
    sprintf(botPath, "%s%s%s", baseDir, BB_DIRSEP, origFilename);
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

sf::RenderWindow* GuiManager::initMainWindow(unsigned int width,
                                         unsigned int height) {
  if (window_ != 0) {
    delete window_;
  }
  window_ = new sf::RenderWindow(sf::VideoMode(width, height), "BerryBots",
                                 sf::Style::Default,
                                 sf::ContextSettings(0, 0, 16, 2, 0));
  return window_;
}

sf::RenderWindow* GuiManager::getMainWindow() {
  return window_;
}

void GuiManager::runNewMatch(char *stagePath, char **teamPaths, int numTeams) {
  deleteMatchConsoles();
  if (!restarting_) {
    saveCurrentMatchSettings(stagePath, teamPaths, numTeams);
  }
  if (engine_ != 0) {
    delete engine_;
    engine_ = 0;
  }
  srand((unsigned int) time(NULL));
  engine_ = new BerryBotsEngine(fileManager_);
  Stage *stage = engine_->getStage();
  char *cacheDir = getCacheDirCopy();
  try {
    engine_->initStage(stagePath, cacheDir);
    engine_->initShips(teamPaths, numTeams, cacheDir);
  } catch (EngineException *e) {
    wxMessageDialog errorMessage(NULL, e->what(),
        "BerryBots engine init failed", wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete engine_;
    engine_ = 0;
    delete cacheDir;
    restarting_ = false;
    newMatchDialog_->Show();
    return;
  }
  delete cacheDir;

  interrupted_ = false;
  paused_ = false;
  newMatchDialog_->Hide();
  packageStageDialog_->Hide();
  packageShipDialog_->Hide();
  gfxHandler_ = new GfxEventHandler();
  stage->addEventHandler((EventHandler*) gfxHandler_);

  viewWidth_ = stage->getWidth() + (STAGE_MARGIN * 2);
  viewHeight_ = stage->getHeight() + (STAGE_MARGIN * 2);
  unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
  unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth - DOCK_SIZE) / viewWidth_,
                             ((double) screenHeight) / viewHeight_));
  unsigned int targetWidth = floor(windowScale * viewWidth_) + DOCK_SIZE;
  unsigned int targetHeight = floor(windowScale * viewHeight_);
  sf::RenderWindow *window = initMainWindow(targetWidth, targetHeight);

  // TODO: If/when SFML getPosition() works, adjust the window position to
  //       keep the whole window on the screen (if necessary). Might be worth
  //       platform-specific implementations using getSystemHandle() if that
  //       doesn't happen in a reasonable timeframe.
  //       We could just set it to (0, 0) or centered on screen every time, but
  //       that seems potentially super annoying to a user - less annoying than
  //       having to move the window occasionally if you switch to a bigger
  //       stage that goes off-screen.

  gfxManager_->initBbGfx(window, viewHeight_, stage, engine_->getTeams(),
                         engine_->getNumTeams(), engine_->getShips(),
                         engine_->getNumShips(), resourcePath());
  gfxManager_->updateView(window, viewWidth_, viewHeight_);
  window->setVisible(true);
  window->clear();
  gfxManager_->drawGame(window, stage, engine_->getShips(),
                        engine_->getNumShips(), engine_->getGameTime(),
                        gfxHandler_, false, false);
  window->display();

  stageConsole_ = new OutputConsole(stage->getName(), menuBarMaker_);
  stageConsole_->Hide();
  teamConsoles_ = new OutputConsole*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    OutputConsole *teamConsole =
        new OutputConsole(engine_->getTeam(x)->name, menuBarMaker_);
    teamConsole->Hide();
    teamConsoles_[x] = teamConsole;
  }
  if (printHandler != 0) {
    delete printHandler;
  }
  printHandler = new GuiPrintHandler(stageConsole_, teamConsoles_,
                                     engine_->getTeams(), numTeams);

  runCurrentMatch();

  while (restarting_) {
    runNewMatch(currentStagePath_, currentTeamPaths_, currentNumTeams_);
  }
}

// TODO: Track and display TPS in GUI.
void GuiManager::runCurrentMatch() {
  interrupted_ = false;
  restarting_ = false;
  sf::RenderWindow *window = getMainWindow();
  try {
    while (window->isOpen() && !interrupted_ && !restarting_ && !quitting_) {
      if (!paused_ && !restarting_ && !engine_->isGameOver()) {
        engine_->processTick();
      }
      
      processMainWindowEvents();
      // TODO: Leaking a bit per bot per match on my MacBook Pro (10.8, Cocoa).
      //       SFML folks seem to think it's in the video drivers.
      //       http://en.sfml-dev.org/forums/index.php?topic=8609.0
      //       Really need to investigate more and/or find a work-around. Also
      //       seeing it under Linux/GTK, to a lesser extent.
      
      if (!interrupted_ && !restarting_ && !quitting_) {
        window->clear();
        gfxManager_->drawGame(window, engine_->getStage(), engine_->getShips(),
                              engine_->getNumShips(), engine_->getGameTime(),
                              gfxHandler_, paused_, engine_->isGameOver());
        window->display();
        
      }
    }
  } catch (EngineException *e) {
    wxMessageDialog errorMessage(NULL, e->what(),
        "BerryBots encountered an error", wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    newMatchDialog_->Show();
    return;
  }

  if (!window->isOpen()) {
    listener_->onAllWindowsClosed();
  }

  // TODO: Display winner / CPU usage in GUI

  if (!interrupted_) {
    gfxManager_->destroyBbGfx();
    delete printHandler;
    printHandler = 0;
    delete engine_;
    engine_ = 0;
    delete gfxHandler_;
    deleteMatchConsoles();
  }
}

void GuiManager::resumeMatch() {
  if (window_ == 0) {
    listener_->onAllWindowsClosed();
  } else {
    if (interrupted_) {
      hideNewMatchDialog();
      hidePackageShipDialog();
      hidePackageStageDialog();
      hidePackagingConsole();
      runCurrentMatch();
    }
    while (restarting_) {
      runNewMatch(currentStagePath_, currentTeamPaths_, currentNumTeams_);
    }
  }
}

void GuiManager::processMainWindowEvents() {
  sf::RenderWindow *window = getMainWindow();
  sf::Event event;
  bool resized = false;
  while (window->pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      window->close();
    }
    if (event.type == sf::Event::Resized && !resized) {
      resized = true;
      gfxManager_->updateView(window, viewWidth_, viewHeight_);
    }
    if (event.type == sf::Event::MouseButtonPressed) {
      gfxManager_->processMouseClick(event.mouseButton.x,
                                     event.mouseButton.y);
    }
    if (event.type == sf::Event::MouseMoved
        || event.type == sf::Event::MouseEntered) {
      gfxManager_->processMouseMoved(event.mouseMove.x,
                                     event.mouseMove.y);
    }
    if (event.type == sf::Event::MouseLeft) {
      gfxManager_->processMouseMoved(-1, -1);
    }
    if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
        case sf::Keyboard::Space:
          togglePause();
          break;
        case sf::Keyboard::BackSpace:
          restartMatch();
          break;
        case sf::Keyboard::N:
        case sf::Keyboard::Escape:
          showNewMatchDialog();
          break;
        default:
          break;
      }
    }

    // On Mac/Cocoa, when using a different Space, the rest of the OS UI slows
    // to a crawl unless you have a frame rate limit set. But the frame rate is
    // smoother if we use vsync instead of a fixed frame rate, so do that when we
    // have focus.
    // TODO: Determine if this is necessary/preferable on Linux/Windows.
    // TODO: Might be better to restrict this to the Space case specifically,
    //       or when window isn't visible to user.
    if (event.type == sf::Event::LostFocus) {
      window->setVerticalSyncEnabled(false);
      window->setFramerateLimit(60);
    } else if (event.type == sf::Event::GainedFocus) {
      window->setVerticalSyncEnabled(true);
      window->setFramerateLimit(0);
    }

    // On Linux/GTK and Windows, the wxWidgets windows freeze while running a
    // match presumably because they only listen for events on the same thread
    // that we're currently occupying to run/draw. Works fine on Mac/Cocoa for
    // whatever reason.
#ifndef __WXOSX__
    wxYield();
#endif
  }
}

void GuiManager::showNewMatchDialog() {
  interrupted_ = true;
  packagingConsole_->Hide();
  newMatchDialog_->Show();
  newMatchDialog_->Raise();
}

void GuiManager::showPackageShipDialog() {
  interrupted_ = true;
  if (!packageShipDialog_->IsShown()) {
    packagingConsole_->Hide();
  }
  packageShipDialog_->Show();
  packageShipDialog_->Raise();
}

void GuiManager::showPackageStageDialog() {
  interrupted_ = true;
  if (!packageStageDialog_->IsShown()) {
    packagingConsole_->Hide();
  }
  packageStageDialog_->Show();
  packageStageDialog_->Raise();
}

void GuiManager::showStageConsole() {
  stageConsole_->Show();
}

void GuiManager::showTeamConsole(int teamIndex) {
  teamConsoles_[teamIndex]->Show();
}

void GuiManager::deleteMatchConsoles() {
  if (stageConsole_ != 0) {
    stageConsole_->Hide();
    delete stageConsole_;
    stageConsole_ = 0;
  }
  if (teamConsoles_ != 0) {
    for (int x = 0; x < currentNumTeams_; x++) {
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

void GuiManager::hidePackagingConsole() {
  packagingConsole_->Hide();
}

void GuiManager::saveCurrentMatchSettings(
    char *stagePath, char **teamPaths, int numTeams) {
  deleteCurrentMatchSettings();
  currentStagePath_ = new char[strlen(stagePath) + 1];
  strcpy(currentStagePath_, stagePath);
  currentTeamPaths_ = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    currentTeamPaths_[x] = new char[strlen(teamPaths[x]) + 1];
    strcpy(currentTeamPaths_[x], teamPaths[x]);
  }
  currentNumTeams_ = numTeams;
}

void GuiManager::deleteCurrentMatchSettings() {
  if (currentStagePath_ != 0) {
    delete currentStagePath_;
    currentStagePath_ = 0;
  }
  if (currentTeamPaths_ != 0) {
    for (int x = 0; x < currentNumTeams_; x++) {
      delete currentTeamPaths_[x];
    }
    delete currentTeamPaths_;
    currentTeamPaths_ = 0;
  }
}

void GuiManager::togglePause() {
  paused_ = !paused_;
}

void GuiManager::restartMatch() {
  restarting_ = true;
}

void GuiManager::quit() {
  quitting_ = true;
}

char* GuiManager::getStageDirCopy() {
  char *stageDir = new char[getStageDir().length() + 1];
  strcpy(stageDir, getStageDir().c_str());
  return stageDir;
}

char* GuiManager::getBotsDirCopy() {
  char *botsDir = new char[getBotsDir().length() + 1];
  strcpy(botsDir, getBotsDir().c_str());
  return botsDir;
}

char* GuiManager::getCacheDirCopy() {
  char *cacheDir = new char[getCacheDir().length() + 1];
  strcpy(cacheDir, getCacheDir().c_str());
  return cacheDir;
}

char* GuiManager::getTmpDirCopy() {
  char *tmpDir = new char[getTmpDir().length() + 1];
  strcpy(tmpDir, getTmpDir().c_str());
  return tmpDir;
}

MatchRunner::MatchRunner(GuiManager *guiManager, char *stageDir,
                         char *botsDir) {
  guiManager_ = guiManager;
  stageDir_ = new char[strlen(stageDir) + 1];
  strcpy(stageDir_, stageDir);
  botsDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsDir_, botsDir);
}

MatchRunner::~MatchRunner() {
  delete stageDir_;
  delete botsDir_;
}

void MatchRunner::startMatch(const char *stageName, char **teamNames,
                             int numTeams) {
  unsigned long stagePathLen =
      strlen(stageDir_) + strlen(BB_DIRSEP) + strlen(stageName);
  char *stagePath = new char[stagePathLen + 1];
  sprintf(stagePath, "%s%s%s", stageDir_, BB_DIRSEP, stageName);
  char **teamPaths = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    unsigned long teamPathLen = strlen(botsDir_) + strlen(BB_DIRSEP)
        + strlen(teamNames[x]);
    char *teamPath = new char[teamPathLen + 1];
    sprintf(teamPath, "%s%s%s", botsDir_, BB_DIRSEP, teamNames[x]);
    teamPaths[x] = teamPath;
  }

  guiManager_->runNewMatch(stagePath, teamPaths, numTeams);
}

void MatchRunner::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadBots();
}

void MatchRunner::cancel() {
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

void ShipPackager::package(const char *botName, const char *version,
                           bool nosrc) {
  char *botsPath = new char[strlen(botsDir_) + strlen(botName) + 2];
  sprintf(botsPath, "%s%s%s", botsDir_, BB_DIRSEP, botName);
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageBot(botsPath, version, cacheDir, tmpDir, nosrc, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageBot(botsPath, version, cacheDir, tmpDir, nosrc,
                               true);
      fileManager_->deleteFromCache(cacheDir, e->what());
    } else {
      refresh = false;
    }
  } catch (std::exception *e) {
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging ship failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
  }
  delete botsPath;
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadBots();
  }
}

void ShipPackager::cancel() {
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

void StagePackager::package(const char *stageName, const char *version,
                            bool nosrc) {
  char *stagePath = new char[strlen(stageDir_) + strlen(stageName) + 2];
  sprintf(stagePath, "%s%s%s", stageDir_, BB_DIRSEP, stageName);
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageStage(stagePath, version, cacheDir, tmpDir, nosrc, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageStage(stagePath, version, cacheDir, tmpDir, nosrc,
                                 true);
      fileManager_->deleteFromCache(cacheDir, e->what());
    } else {
      refresh = false;
    }
  } catch (std::exception *e) {
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging stage failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
  }
  delete stagePath;
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadStages();
  }
}

void StagePackager::cancel() {
  guiManager_->resumeMatch();
}

PackageReporter::PackageReporter(OutputConsole *packagingConsole) {
  packagingConsole_ = packagingConsole;
}

void PackageReporter::packagingComplete(
    char **sourceFiles, int numFiles, bool nosrc, const char *destinationFile) {
  packagingConsole_->clear();
  packagingConsole_->Show();
  if (nosrc) {
    packagingConsole_->println(
        "The following files were compiled and packaged as Lua bytecode:");
  } else {
    packagingConsole_->println(
        "The following files were packaged as source code:");
  }
  for (int x = 0; x < numFiles; x++) {
    if (sourceFiles[x] != 0) {
      packagingConsole_->print("  ");
      packagingConsole_->println(sourceFiles[x]);
    }
  }
  packagingConsole_->println();
  packagingConsole_->print("Saved to: ");
  packagingConsole_->println(destinationFile);
  packagingConsole_->Raise();
}

ViewListener::ViewListener(GuiManager *guiManager) {
  guiManager_ = guiManager;
}

void ViewListener::onNewMatch() {
  guiManager_->showNewMatchDialog();
}

void ViewListener::onPackageShip() {
  guiManager_->showPackageShipDialog();
}

void ViewListener::onPackageStage() {
  guiManager_->showPackageStageDialog();
}

void ViewListener::onStageClick() {
  guiManager_->showStageConsole();
}

void ViewListener::onTeamClick(int teamIndex) {
  guiManager_->showTeamConsole(teamIndex);
}

void ViewListener::onPauseUnpause() {
  guiManager_->togglePause();
}

void ViewListener::onRestart() {
  guiManager_->restartMatch();
}
