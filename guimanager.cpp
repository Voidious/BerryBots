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

GuiManager::GuiManager(GuiListener *listener) {
  listener_ = listener;
  stageBaseDir_ = botsBaseDir_ = 0;
  reloadBaseDirs();

  window_ = 0;
  previewWindow_ = 0;
  zipper_ = new GuiZipper();
  fileManager_ = new FileManager(zipper_);
  menuBarMaker_ = new MenuBarMaker();
  packagingConsole_ = new OutputConsole("Packaging Details", menuBarMaker_);
  errorConsole_ = new OutputConsole("Error Console", menuBarMaker_);
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
  previewGfxManager_ = new GfxManager(false);
  viewListener_ = new ViewListener(this);
  gfxManager_->setListener(viewListener_);
  packageReporter_ = new PackageReporter(packagingConsole_);
  fileManager_->setListener(packageReporter_);
  printStateListener_ = 0;
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
  tpsFactor_ = 1;
  nextDrawTime_ = 1;
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
  if (previewWindow_ != 0) {
    delete previewWindow_;
  }
  delete gfxManager_;
  delete viewListener_;
  delete zipper_;
  delete fileManager_;
  delete newMatchListener_;
  delete shipPackager_;
  delete stagePackager_;
  delete packageReporter_;
  if (printStateListener_ != 0) {
    delete printStateListener_;
    printStateListener_ = 0;
  }
}

void GuiManager::setBaseDirs(const char *stagesBaseDir,
                             const char *botsBaseDir) {
  if (stageBaseDir_ != 0) {
    delete stageBaseDir_;
  }
  stageBaseDir_ = new char[strlen(stagesBaseDir) + 1];
  strcpy(stageBaseDir_, stagesBaseDir);

  if (botsBaseDir_ != 0) {
    delete botsBaseDir_;
  }
  botsBaseDir_ = new char[strlen(botsBaseDir) + 1];
  strcpy(botsBaseDir_, botsBaseDir);
}

void GuiManager::reloadBaseDirs() {
  setBaseDirs(getStageDir().c_str(), getBotsDir().c_str());
}

void GuiManager::loadStages() {
  newMatchDialog_->clearStages();
  packageStageDialog_->clearItems();
  loadStagesFromDir(stageBaseDir_);
}

void GuiManager::loadStagesFromDir(const char *loadDir) {
  BerryBotsEngine engine(fileManager_);
  loadItemsFromDir(stageBaseDir_, loadDir, ITEM_STAGE, packageStageDialog_,
                   &engine);
}

bool GuiManager::isValidStageFile(const char *srcFilename,
                                  BerryBotsEngine *engine) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       stages? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(srcFilename)
      || fileManager_->isZipFilename(srcFilename)) {
    char *cacheDir = getCacheDirCopy();
    char *stageDir = 0;
    char *stageFilename = 0;
    try {
      fileManager_->loadStageFileData(stageBaseDir_, srcFilename, &stageDir,
                                      &stageFilename, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete cacheDir;
      if (stageDir != 0) {
        delete stageDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      return false;
    } catch (ZipperException *ze) {
      delete cacheDir;
      if (stageDir != 0) {
        delete stageDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      errorConsole_->println(ze->what());
      wxMessageDialog errorMessage(NULL, ze->what(), "Unzip failure",
                                   wxOK | wxICON_EXCLAMATION);
      errorMessage.ShowModal();
      return false;
    }
    delete cacheDir;
    lua_State *stageState;
    initStageState(&stageState, stageDir);

    if (luaL_loadfile(stageState, stageFilename)
        || engine->callUserLuaCode(stageState, 0, "", PCALL_VALIDATE)) {
      logErrorMessage(stageState, "Problem loading stage: %s");
      lua_close(stageState);
      delete stageDir;
      delete stageFilename;
      return false;
    }

    lua_getglobal(stageState, "configure");
    if (lua_isnil(stageState, -1)) {
      lua_close(stageState);
      delete stageDir;
      delete stageFilename;
      return false;
    }

    lua_close(stageState);
    delete stageDir;
    delete stageFilename;
    return true;
  }
  return false;
}

void GuiManager::loadBots() {
  newMatchDialog_->clearBots();
  packageShipDialog_->clearItems();
  loadBotsFromDir(botsBaseDir_);
  newMatchDialog_->removeStaleLoadedBots();
}

void GuiManager::loadBotsFromDir(const char *loadDir) {
  BerryBotsEngine engine(fileManager_);
  loadItemsFromDir(botsBaseDir_, loadDir, ITEM_BOT, packageShipDialog_,
                   &engine);
}

bool GuiManager::isValidBotFile(const char *srcFilename,
                                BerryBotsEngine *engine) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       ships? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(srcFilename)
      || fileManager_->isZipFilename(srcFilename)) {
    char *cacheDir = getCacheDirCopy();
    char *botDir = 0;
    char *botFilename = 0;
    try {
      fileManager_->loadBotFileData(botsBaseDir_, srcFilename, &botDir,
                                    &botFilename, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete cacheDir;
      if (botDir != 0) {
        delete botDir;
      }
      if (botFilename != 0) {
        delete botFilename;
      }
      return false;
    } catch (ZipperException *ze) {
      delete cacheDir;
      if (botDir != 0) {
        delete botDir;
      }
      if (botFilename != 0) {
        delete botFilename;
      }
      errorConsole_->println(ze->what());
      wxMessageDialog errorMessage(NULL, ze->what(), "Unzip failure",
                                   wxOK | wxICON_EXCLAMATION);
      errorMessage.ShowModal();
      return false;
    }
    delete cacheDir;
    lua_State *shipState;
    initShipState(&shipState, botDir);

    if (luaL_loadfile(shipState, botFilename)
        || engine->callUserLuaCode(shipState, 0, "", PCALL_VALIDATE)) {
      logErrorMessage(shipState, "Problem loading ship: %s");
      lua_close(shipState);
      delete botDir;
      delete botFilename;
      return false;
    }

    lua_getglobal(shipState, "configure");
    lua_getglobal(shipState, "init");
    if (lua_isnil(shipState, -1) || !lua_isnil(shipState, -2)) {
      lua_close(shipState);
      delete botDir;
      delete botFilename;
      return false;
    }
    
    lua_close(shipState);
    delete botDir;
    delete botFilename;
    return true;
  }
  return false;
}

void GuiManager::loadItemsFromDir(const char *baseDir, const char *loadDir,
    int itemType, PackageDialog *packageDialog, BerryBotsEngine *engine) {
  platformstl::readdir_sequence dir(loadDir,
      platformstl::readdir_sequence::files
          | platformstl::readdir_sequence::directories);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    char *filePath = FileManager::getFilePath(loadDir, filename);
    if (FileManager::isDirectory(filePath)) {
      loadItemsFromDir(baseDir, filePath, itemType, packageDialog, engine);
    } else {
      char *relativeFilename = &(filePath[strlen(baseDir) + 1]);
      bool valid = false;
      if (itemType == ITEM_BOT && isValidBotFile(relativeFilename, engine)) {
        newMatchDialog_->addBot(relativeFilename);
        valid = true;
      } else if (itemType == ITEM_STAGE && isValidStageFile(relativeFilename,
                                                            engine)) {
        newMatchDialog_->addStage(relativeFilename);
        valid = true;
      }
      if (valid && fileManager_->isLuaFilename(filename)) {
        packageDialog->addItem(relativeFilename);
      }
    }
    delete filePath;
  }
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

void GuiManager::runNewMatch(const char *stageName, char **teamNames,
                             int numTeams) {
  tpsFactor_ = 1;
  nextDrawTime_ = 1;
  sf::RenderWindow *window;
#ifdef __WXOSX__
  window = initMainWindow(1200, 800);
#endif
  deleteMatchConsoles();
  if (!restarting_) {
    saveCurrentMatchSettings(stageName, teamNames, numTeams);
  }
  if (engine_ != 0) {
    delete engine_;
    engine_ = 0;
  }

  stageConsole_ = new OutputConsole(stageName, menuBarMaker_);
  stageConsole_->print("Stage control program loaded: ");
  stageConsole_->println(stageName);
  stageConsole_->Hide();
  
  if (printHandler != 0) {
    delete printHandler;
    printHandler = 0;
  }
  if (printStateListener_ != 0) {
    delete printStateListener_;
    printStateListener_ = 0;
  }
  GuiPrintHandler *guiPrintHandler =
      new GuiPrintHandler(stageConsole_, numTeams, menuBarMaker_);
  printStateListener_ = new PrintStateListener(guiPrintHandler);
  printHandler = guiPrintHandler;

  srand((unsigned int) time(NULL));
  engine_ = new BerryBotsEngine(fileManager_);
  engine_->setListener(printStateListener_);
  Stage *stage = engine_->getStage();
  char *cacheDir = getCacheDirCopy();
  try {
    engine_->initStage(stageBaseDir_, stageName, cacheDir);
    engine_->initShips(botsBaseDir_, teamNames, numTeams, cacheDir);
    teamConsoles_ = guiPrintHandler->getTeamConsoles();

    for (int x = 0; x < numTeams; x++) {
      teamConsoles_[x]->println();
    }
    stageConsole_->println();
  } catch (EngineException *e) {
#ifdef __WXOSX__
    delete window_;
    window_ = 0;
#endif
    errorConsole_->println(e->what());
    wxMessageDialog errorMessage(NULL, e->what(),
        "BerryBots engine init failed", wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete printHandler;
    printHandler = 0;
    delete printStateListener_;
    printStateListener_ = 0;
    delete engine_;
    engine_ = 0;
    delete cacheDir;
    restarting_ = false;
    newMatchDialog_->Show();
    return;
  }
  delete cacheDir;

  viewWidth_ = stage->getWidth() + (STAGE_MARGIN * 2);
  viewHeight_ = stage->getHeight() + (STAGE_MARGIN * 2);
  unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
  unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth - DOCK_SIZE) / viewWidth_,
                             ((double) screenHeight) / viewHeight_));
  unsigned int targetWidth = floor(windowScale * viewWidth_) + DOCK_SIZE;
  unsigned int targetHeight = floor(windowScale * viewHeight_);
#ifdef __WXOSX__
  window->setSize(sf::Vector2u(targetWidth, targetHeight));
  sf::Vector2i pos = window->getPosition();
  int x = std::max(0, std::min((int) (screenWidth - targetWidth), pos.x));
  int y = std::max(0, std::min((int) (screenHeight - targetHeight), pos.y));
  window->setPosition(sf::Vector2i(x, y));
#else
  window = initMainWindow(targetWidth, targetHeight);
#endif

  interrupted_ = false;
  paused_ = false;
  newMatchDialog_->Hide();
  packageStageDialog_->Hide();
  packageShipDialog_->Hide();
  gfxHandler_ = new GfxEventHandler();
  stage->addEventHandler((EventHandler*) gfxHandler_);

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
                        gfxHandler_, false, false, engine_->getWinnerName());
  window->display();

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
        while (engine_->getGameTime() < nextDrawTime_) {
          engine_->processTick();
        }
      }
      
      while (!interrupted_ && !restarting_ && !quitting_
             && nextDrawTime_ <= engine_->getGameTime()) {
        processMainWindowEvents(window, gfxManager_, viewWidth_, viewHeight_);
        window->clear();
        gfxManager_->drawGame(window, engine_->getStage(), engine_->getShips(),
                              engine_->getNumShips(), engine_->getGameTime(),
                              gfxHandler_, paused_, engine_->isGameOver(),
                              engine_->getWinnerName());
        window->display();
        if (!paused_) {
          nextDrawTime_ += tpsFactor_;
        }
      }
    }
  } catch (EngineException *e) {
    errorConsole_->println(e->what());
    wxMessageDialog errorMessage(NULL, e->what(),
        "BerryBots encountered an error", wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    newMatchDialog_->Show();
    return;
  }

  if (!window->isOpen()) {
    listener_->onAllWindowsClosed();
  }

  // TODO: Display CPU usage in GUI

  if (!interrupted_) {
    gfxManager_->destroyBbGfx();
    delete printHandler;
    printHandler = 0;
    delete printStateListener_;
    printStateListener_ = 0;
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
      gfxManager_->hideKeyboardShortcuts();
      hideNewMatchDialog();
      hidePackageShipDialog();
      hidePackageStageDialog();
      hidePackagingConsole();
      hideErrorConsole();
      runCurrentMatch();
    }
    while (restarting_) {
      runNewMatch(currentStagePath_, currentTeamPaths_, currentNumTeams_);
    }
  }
}

void GuiManager::processMainWindowEvents(sf::RenderWindow *window,
    GfxManager *gfxManager, int viewWidth, int viewHeight) {
  sf::Event event;
  bool resized = false;
  while (window->pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      window->close();
    }
    if (event.type == sf::Event::Resized && !resized) {
      resized = true;
      gfxManager->updateView(window, viewWidth, viewHeight);
    }
    if (event.type == sf::Event::MouseButtonPressed) {
      gfxManager->processMouseDown(event.mouseButton.x, event.mouseButton.y);
    }
    if (event.type == sf::Event::MouseButtonReleased) {
      gfxManager->processMouseUp(event.mouseButton.x, event.mouseButton.y);
    }
    if (event.type == sf::Event::MouseMoved
        || event.type == sf::Event::MouseEntered) {
      gfxManager->processMouseMoved(event.mouseMove.x, event.mouseMove.y);
    }
    if (event.type == sf::Event::MouseLeft) {
      gfxManager->processMouseMoved(-1, -1);
    }
    if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
        case sf::Keyboard::Space:
          togglePause();
          break;
        case sf::Keyboard::BackSpace:
          restartMatch();
          break;
        case sf::Keyboard::Escape:
          showNewMatchDialog();
          break;
#ifdef __WXOSX__
        case sf::Keyboard::LSystem:
        case sf::Keyboard::RSystem:
          gfxManager->showKeyboardShortcuts();
          break;
#else
        case sf::Keyboard::LAlt:
        case sf::Keyboard::RAlt:
          gfxManager->showKeyboardShortcuts();
          break;
#endif
        case sf::Keyboard::N:
          showNewMatchDialog();
          break;
        case sf::Keyboard::P:
          showPackageShipDialog();
          break;
        case sf::Keyboard::T:
          showPackageStageDialog();
          break;
        default:
          break;
      }
    }

    if (event.type == sf::Event::KeyReleased) {
      switch (event.key.code) {
#ifdef __WXOSX__
        case sf::Keyboard::LSystem:
        case sf::Keyboard::RSystem:
          gfxManager->hideKeyboardShortcuts();
          break;
#else
        case sf::Keyboard::LAlt:
        case sf::Keyboard::RAlt:
          gfxManager->hideKeyboardShortcuts();
          break;
#endif
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
    bool defaultTps = (abs(tpsFactor_ - 1) < 0.001);
    if (event.type == sf::Event::LostFocus) {
      window->setVerticalSyncEnabled(false);
      window->setFramerateLimit(60);
    } else if (event.type == sf::Event::GainedFocus && defaultTps) {
      window->setVerticalSyncEnabled(true);
      window->setFramerateLimit(0);
    }
  }

  // On Linux/GTK and Windows, the wxWidgets windows don't get events while
  // this thread has control unless we manually wxYield each frame. Seems to be
  // unnecessary on Mac/Cocoa.
#ifndef __WXOSX__
  wxYield();
#endif
}

void GuiManager::processPreviewWindowEvents(sf::RenderWindow *window,
    GfxManager *gfxManager, int viewWidth, int viewHeight) {
  sf::Event event;
  bool resized = false;
  while (window->pollEvent(event)) {
    if (event.type == sf::Event::Closed
        || (event.type == sf::Event::KeyPressed
            && event.key.code == sf::Keyboard::Escape)
        || (event.type == sf::Event::LostFocus)) {
      window->close();
    }
    if (event.type == sf::Event::Resized && !resized) {
      resized = true;
      gfxManager->updateView(window, viewWidth, viewHeight);
    }
  }

  // On Linux/GTK and Windows, the wxWidgets windows don't get events while
  // this thread has control unless we manually wxYield each frame. Seems to be
  // unnecessary on Mac/Cocoa.
#ifndef __WXOSX__
  wxYield();
#endif
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
  stageConsole_->Raise();
}

void GuiManager::showTeamConsole(int teamIndex) {
  teamConsoles_[teamIndex]->Show();
  teamConsoles_[teamIndex]->Raise();
}

void GuiManager::showErrorConsole() {
  errorConsole_->Show();
  errorConsole_->Raise();
}

void GuiManager::showStagePreview(const char *stageName) {
  // TODO: this is really going to screw up GfxManager until all the globals
  //       are removed
  GfxEventHandler *gfxHandler = new GfxEventHandler();

  BerryBotsEngine *engine = new BerryBotsEngine(fileManager_);
  char *cacheDir = getCacheDirCopy();
  engine->initStage(stageBaseDir_, stageName, cacheDir);
  
  Stage *stage = engine->getStage();
  unsigned int viewWidth = stage->getWidth() + (2 * STAGE_MARGIN);
  unsigned int viewHeight = stage->getHeight() + (2 * STAGE_MARGIN);
  unsigned int screenWidth = 600;
  unsigned int screenHeight = 600;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth) / viewWidth,
                             ((double) screenHeight) / viewHeight));
  unsigned int targetWidth = floor(windowScale * viewWidth);
  unsigned int targetHeight = floor(windowScale * viewHeight);
  if (previewWindow_ != 0) {
    delete previewWindow_;
  }
  previewWindow_ = new sf::RenderWindow(
      sf::VideoMode(targetWidth, targetHeight), stageName, sf::Style::Default,
      sf::ContextSettings(0, 0, 16, 2, 0));

  Team **teams = new Team*[1];
  teams[0] = new Team;
  strcpy(teams[0]->name, "PreviewTeam");
  Ship **ships = new Ship*[1];
  Ship *ship = new Ship;
  ShipProperties *properties = new ShipProperties;
  properties->shipR = properties->shipG = properties->shipB = 255;
  properties->laserR = properties->laserB = 0;
  properties->laserG = 255;
  properties->thrusterR = 255;
  properties->thrusterG = properties->thrusterB = 0;
  strcpy(properties->name, "PreviewShip");
  ship->properties = properties;
  ship->thrusterAngle = ship->thrusterForce = 0;
  Point2D *start = stage->getStart();
  ship->x = start->getX();
  ship->y = start->getY();
  ship->alive = true;
  ship->showName = ship->energyEnabled = false;
  ships[0] = ship;

  previewGfxManager_->initBbGfx(previewWindow_, viewHeight, stage, teams, 1,
                                ships, 1, resourcePath());
  previewGfxManager_->updateView(previewWindow_, viewWidth, viewHeight);

  while (previewWindow_->isOpen()) {
    processPreviewWindowEvents(previewWindow_, previewGfxManager_, viewWidth,
                               viewHeight);
    previewWindow_->clear();
    previewGfxManager_->drawGame(previewWindow_, stage, ships, 1, 0, gfxHandler,
                                 false, false, 0);
    previewWindow_->display();
  }

  previewGfxManager_->destroyBbGfx();
  delete gfxHandler;
  delete engine;
  delete properties;
  delete ships[0];
  delete teams[0];
  delete ships;
  delete teams;

  newMatchDialog_->Show();
  newMatchDialog_->Raise();
}

void GuiManager::deleteMatchConsoles() {
  if (stageConsole_ != 0) {
    stageConsole_->Hide();
    delete stageConsole_;
    stageConsole_ = 0;
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

void GuiManager::hideErrorConsole() {
  errorConsole_->Hide();
}

void GuiManager::newMatchInitialFocus() {
  newMatchDialog_->initialFocus();
}

void GuiManager::packageShipInitialFocus() {
  packageShipDialog_->initialFocus();
}

void GuiManager::packageStageInitialFocus() {
  packageStageDialog_->initialFocus();
}

void GuiManager::saveCurrentMatchSettings(
    const char *stagePath, char **teamPaths, int numTeams) {
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

void GuiManager::setTpsFactor(double tpsFactor) {
  tpsFactor_ = tpsFactor;
  int newTps = (int) (tpsFactor_ * 72);
  paused_ = (newTps == 0);

  sf::RenderWindow *window = getMainWindow();
  bool defaultTps = (abs(tpsFactor_ - 1) < 0.01);
  if (defaultTps) {
    window->setVerticalSyncEnabled(true);
    window->setFramerateLimit(0);
  } else {
    window->setVerticalSyncEnabled(false);
    window->setFramerateLimit(60);
  }
}

void GuiManager::quit() {
  quitting_ = true;
}

void GuiManager::logErrorMessage(lua_State *L, const char *formatString) {
  const char *luaMessage = lua_tostring(L, -1);
  int messageLen = (int) (strlen(formatString) + strlen(luaMessage) - 2);
  char *errorMessage = new char[messageLen + 1];
  sprintf(errorMessage, formatString, luaMessage);
  errorConsole_->println(errorMessage);
  delete errorMessage;
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
  guiManager_->runNewMatch(stageName, teamNames, numTeams);
}

void MatchRunner::previewStage(const char *stageName) {
  guiManager_->showStagePreview(stageName);
}

void MatchRunner::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadBots();
}

void MatchRunner::cancel() {
  guiManager_->resumeMatch();
}

void MatchRunner::reloadBaseDirs() {
  guiManager_->reloadBaseDirs();
  refreshFiles();
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
                           bool obfuscate) {
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageBot(botsDir_, botName, version, cacheDir, tmpDir,
                             obfuscate, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageBot(botsDir_, botName, version, cacheDir, tmpDir,
                               obfuscate, true);
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
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadBots();
  }
}

void ShipPackager::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadBots();
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
                            bool obfuscate) {
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageStage(stageDir_, stageName, version, cacheDir, tmpDir,
                               obfuscate, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageStage(stageDir_, stageName, version, cacheDir,
                                 tmpDir, obfuscate, true);
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
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadStages();
  }
}

void StagePackager::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadBots();
}

void StagePackager::cancel() {
  guiManager_->resumeMatch();
}

PackageReporter::PackageReporter(OutputConsole *packagingConsole) {
  packagingConsole_ = packagingConsole;
}

void PackageReporter::packagingComplete(char **sourceFiles, int numFiles,
    bool obfuscate, const char *destinationFile) {
  packagingConsole_->clear();
  packagingConsole_->Show();
  if (obfuscate) {
    packagingConsole_->println(
        "The following files were packaged as obfuscated source code:");
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

PrintStateListener::PrintStateListener(GuiPrintHandler *guiPrintHandler) {
  guiPrintHandler_ = guiPrintHandler;
}

void PrintStateListener::newTeamState(lua_State *teamState,
                                      const char *filename) {
  guiPrintHandler_->registerTeam(teamState, filename);
}

void PrintStateListener::setTeamName(lua_State *teamState,
                                     const char *filename) {
  guiPrintHandler_->setTeamName(teamState, filename);
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

void ViewListener::onTpsChange(double tpsFactor) {
  guiManager_->setTpsFactor(tpsFactor);
}
