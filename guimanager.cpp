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
  stagesBaseDir_ = shipsBaseDir_ = 0;
  newMatchDialog_ = 0;
  srand((unsigned int) time(NULL));
  reloadBaseDirs();

  window_ = 0;
  previewWindow_ = 0;
  zipper_ = new GuiZipper();
  fileManager_ = new FileManager(zipper_);
  menuBarMaker_ = new MenuBarMaker();
  packagingConsole_ = new OutputConsole("Packaging Details", false,
                                        menuBarMaker_);
  errorConsole_ = new OutputConsole("Error Console", false, menuBarMaker_);
  previewConsole_ = new OutputConsole("Description", false, menuBarMaker_);
  previewConsole_->setCloseOnSpace();
  previewConsoleListener_ = new PreviewConsoleListener(this);
  previewConsole_->setListener(previewConsoleListener_);
  packagingConsole_->SetPosition(wxPoint(150, 100));
  newMatchListener_ = new MatchStarter(this, stagesBaseDir_, shipsBaseDir_);
  newMatchDialog_ = new NewMatchDialog(newMatchListener_, menuBarMaker_);
  shipPackager_ = new ShipPackager(this, fileManager_, packagingConsole_,
                                   shipsBaseDir_);
  packageShipDialog_ = new PackageShipDialog(shipPackager_, menuBarMaker_);
  stagePackager_ = new StagePackager(this, fileManager_, packagingConsole_,
                                     stagesBaseDir_, shipsBaseDir_);
  packageStageDialog_ = new PackageStageDialog(stagePackager_, menuBarMaker_);
  loadStages();
  loadShips();

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
  consoleHandler_ = new ConsoleEventHandler(this);
  engine_ = 0;
  gfxHandler_ = 0;
  currentStagePath_ = 0;
  currentTeamPaths_ = 0;
  currentNumTeams_ = 0;
  interrupted_ = false;
  paused_ = false;
  restarting_ = false;
  quitting_ = false;
  previewing_ = false;
  closingPreview_ = false;
  tpsFactor_ = 1;
  nextDrawTime_ = 1;

#ifdef __WINDOWS__
  windowIcon_.loadFromFile(BBICON_32);
#elif defined(__WXGTK__)
  windowIcon_.loadFromFile(BBICON_128);
#endif
}

GuiManager::~GuiManager() {
  deleteStageConsole();
  deleteCurrentMatchSettings();
  delete newMatchDialog_;
  delete packageShipDialog_;
  delete packageStageDialog_;
  delete packagingConsole_;
  delete errorConsole_;
  delete previewConsole_;
  delete previewConsoleListener_;
  delete menuBarMaker_;
  delete stagesBaseDir_;
  delete shipsBaseDir_;
  if (engine_ != 0) {
    delete engine_;
  }
  if (window_ != 0) {
    delete window_;
  }
  if (previewWindow_ != 0) {
    delete previewWindow_;
  }
  if (gfxHandler_ != 0) {
    delete gfxHandler_;
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
  if (printHandler != 0) {
    delete printHandler;
    printHandler = 0;
  }
  delete consoleHandler_;
}

void GuiManager::setBaseDirs(const char *stagesBaseDir,
                             const char *shipsBaseDir) {
  if (stagesBaseDir_ != 0) {
    delete stagesBaseDir_;
  }
  stagesBaseDir_ = new char[strlen(stagesBaseDir) + 1];
  strcpy(stagesBaseDir_, stagesBaseDir);

  if (shipsBaseDir_ != 0) {
    delete shipsBaseDir_;
  }
  shipsBaseDir_ = new char[strlen(shipsBaseDir) + 1];
  strcpy(shipsBaseDir_, shipsBaseDir);

  if (newMatchDialog_ != 0) {
    newMatchDialog_->onSetBaseDirs();
  }
}

void GuiManager::reloadBaseDirs() {
  setBaseDirs(getStagesDir().c_str(), getShipsDir().c_str());
}

void GuiManager::loadStages() {
  newMatchDialog_->clearStages();
  packageStageDialog_->clearItems();
  loadStagesFromDir(stagesBaseDir_);
}

void GuiManager::loadStagesFromDir(const char *loadDir) {
  BerryBotsEngine engine(fileManager_);
  loadItemsFromDir(stagesBaseDir_, loadDir, ITEM_STAGE, packageStageDialog_,
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
    char *stagesDir = 0;
    char *stageFilename = 0;
    try {
      fileManager_->loadStageFileData(stagesBaseDir_, srcFilename, &stagesDir,
                                      &stageFilename, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete cacheDir;
      if (stagesDir != 0) {
        delete stagesDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      delete fnfe;
      return false;
    } catch (ZipperException *ze) {
      delete cacheDir;
      if (stagesDir != 0) {
        delete stagesDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      errorConsole_->print(srcFilename);
      errorConsole_->print(": ");
      errorConsole_->println(ze->what());
      wxMessageDialog errorMessage(NULL, ze->what(), "Unzip failure",
                                   wxOK | wxICON_EXCLAMATION);
      errorMessage.ShowModal();
      delete ze;
      return false;
    } catch (PackagedSymlinkException *pse) {
      delete cacheDir;
      if (stagesDir != 0) {
        delete stagesDir;
      }
      if (stageFilename != 0) {
        delete stageFilename;
      }
      errorConsole_->print(srcFilename);
      errorConsole_->print(": ");
      errorConsole_->println(pse->what());
      delete pse;
      return false;
    }
    delete cacheDir;
    lua_State *stageState;
    initStageState(&stageState, stagesDir);

    if (luaL_loadfile(stageState, stageFilename)
        || engine->callUserLuaCode(stageState, 0, "", PCALL_VALIDATE)) {
      logErrorMessage(stageState, "Problem loading stage: %s");
      lua_close(stageState);
      delete stagesDir;
      delete stageFilename;
      return false;
    }

    lua_getglobal(stageState, "configure");
    if (lua_isnil(stageState, -1)) {
      lua_close(stageState);
      delete stagesDir;
      delete stageFilename;
      return false;
    }

    lua_close(stageState);
    delete stagesDir;
    delete stageFilename;
    return true;
  }
  return false;
}

void GuiManager::loadShips() {
  newMatchDialog_->clearShips();
  packageShipDialog_->clearItems();
  loadShipsFromDir(shipsBaseDir_);
  newMatchDialog_->removeStaleLoadedShips();
}

void GuiManager::loadShipsFromDir(const char *loadDir) {
  BerryBotsEngine engine(fileManager_);
  loadItemsFromDir(shipsBaseDir_, loadDir, ITEM_SHIP, packageShipDialog_,
                   &engine);
}

bool GuiManager::isValidShipFile(const char *srcFilename,
                                BerryBotsEngine *engine) {
  // TODO: Is this too slow? Should we keep this list in the cache so we don't
  //       have to do this on every startup / refresh - at least for packaged
  //       ships? In fact, just the presence in the cache could be considered
  //       a sign of validity.
  // TODO: Move this out of the GUI code.
  if (fileManager_->isLuaFilename(srcFilename)
      || fileManager_->isZipFilename(srcFilename)) {
    char *cacheDir = getCacheDirCopy();
    char *shipDir = 0;
    char *shipFilename = 0;
    try {
      fileManager_->loadShipFileData(shipsBaseDir_, srcFilename, &shipDir,
                                     &shipFilename, cacheDir);
    } catch (FileNotFoundException *fnfe) {
      // Only possible if user deletes file from disk after we find it on disk
      // but before we validate it. Seems safe to fail silently.
      delete cacheDir;
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename;
      }
      delete fnfe;
      return false;
    } catch (ZipperException *ze) {
      delete cacheDir;
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename;
      }
      errorConsole_->print(srcFilename);
      errorConsole_->print(": ");
      errorConsole_->println(ze->what());
      wxMessageDialog errorMessage(NULL, ze->what(), "Unzip failure",
                                   wxOK | wxICON_EXCLAMATION);
      errorMessage.ShowModal();
      delete ze;
      return false;
    } catch (PackagedSymlinkException *pse) {
      delete cacheDir;
      if (shipDir != 0) {
        delete shipDir;
      }
      if (shipFilename != 0) {
        delete shipFilename;
      }
      errorConsole_->print(srcFilename);
      errorConsole_->print(": ");
      errorConsole_->println(pse->what());
      delete pse;
      return false;
    }
    delete cacheDir;
    lua_State *shipState;
    initShipState(&shipState, shipDir);

    if (luaL_loadfile(shipState, shipFilename)
        || engine->callUserLuaCode(shipState, 0, "", PCALL_VALIDATE)) {
      logErrorMessage(shipState, "Problem loading ship: %s");
      lua_close(shipState);
      delete shipDir;
      delete shipFilename;
      return false;
    }

    lua_getglobal(shipState, "configure");
    lua_getglobal(shipState, "init");
    if (lua_isnil(shipState, -1) || !lua_isnil(shipState, -2)) {
      lua_close(shipState);
      delete shipDir;
      delete shipFilename;
      return false;
    }
    
    lua_close(shipState);
    delete shipDir;
    delete shipFilename;
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
      if (itemType == ITEM_SHIP && isValidShipFile(relativeFilename, engine)) {
        newMatchDialog_->addShip(relativeFilename);
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
    sf::RenderWindow *oldWindow = window_;
    window_ = 0;
    delete oldWindow;
  }

  window_ = new sf::RenderWindow(sf::VideoMode(width, height), "BerryBots",
      sf::Style::Default, sf::ContextSettings(0, 0, 16, 2, 0));

#ifdef __WINDOWS__
  window_->setIcon(32, 32, windowIcon_.getPixelsPtr());
#elif defined(__WXGTK__)
  window_->setIcon(128, 128, windowIcon_.getPixelsPtr());
#endif

  return window_;
}

sf::RenderWindow* GuiManager::initPreviewWindow(unsigned int width,
                                                unsigned int height) {
  if (previewWindow_ != 0) {
    sf::RenderWindow *oldWindow = previewWindow_;
    previewWindow_ = 0;
    delete oldWindow;
  }

  previewWindow_ = new sf::RenderWindow(sf::VideoMode(width, height), "Preview",
      sf::Style::Default, sf::ContextSettings(0, 0, 16, 2, 0));

#ifdef __WINDOWS__
  previewWindow_->setIcon(32, 32, windowIcon_.getPixelsPtr());
#elif defined(__WXGTK__)
  previewWindow_->setIcon(128, 128, windowIcon_.getPixelsPtr());
#endif

  return previewWindow_;
}

sf::RenderWindow* GuiManager::getMainWindow() {
  return window_;
}

void GuiManager::startMatch(const char *stageName, char **teamNames,
                            int numUserTeams) {
  do {
    runNewMatch(stageName, teamNames, numUserTeams);
  } while (restarting_);
}

void GuiManager::runNewMatch(const char *stageName, char **teamNames,
                             int numUserTeams) {
  tpsFactor_ = 1;
  nextDrawTime_ = 1;

  sf::RenderWindow *window;
  bool maintainWindowProperties = false;
  sf::Vector2i prevPosition;
  double prevScale = 1.0;
  if (window_ != 0) {
#ifndef __WXGTK__
    // sf::Window::getPosition just doesn't work on Linux/GTK, so don't try,
    // which at least lets the window manager position it to be fully on screen.
    prevPosition = window_->getPosition();
    if (restarting_) {
      prevScale = ((double) (window_->getSize().x - DOCK_SIZE)) / viewWidth_;
    }
    maintainWindowProperties = true;
#endif
  }
  
#ifdef __WXOSX__
  // On Mac OS X, we need to initialize before the wxWidgets stuff below or we
  // hit some unexplainable crashes when we delete an SFML window. I don't know
  // why, I've merely devised a work-around. Judging from some SFML forum
  // threads, it sounds likely to be an issue with nightmare-ish video drivers.
  window = initMainWindow(1200, 800);
#endif

  if (!restarting_) {
    saveCurrentMatchSettings(stageName, teamNames, numUserTeams);
  }
  if (engine_ != 0) {
    delete engine_;
    engine_ = 0;
  }

  GuiPrintHandler *guiPrintHandler;
  if (restarting_) {
    stageConsole_->clear();
    guiPrintHandler = (GuiPrintHandler*) printHandler;
    guiPrintHandler->restartMode();
  } else {
    deleteStageConsole();
    stageConsole_ = new OutputConsole(stageName, true, menuBarMaker_);
    stageConsole_->Hide();

    if (printHandler != 0) {
      delete printHandler;
      printHandler = 0;
    }
    if (printStateListener_ != 0) {
      delete printStateListener_;
      printStateListener_ = 0;
    }
    guiPrintHandler = new GuiPrintHandler(stageConsole_, menuBarMaker_);
    printStateListener_ = new PrintStateListener(guiPrintHandler);
    printHandler = guiPrintHandler;
  }
  stageConsole_->print("== Stage control program loaded: ");
  stageConsole_->println(stageName);

  engine_ = new BerryBotsEngine(fileManager_);
  engine_->setListener(printStateListener_);
  Stage *stage = engine_->getStage();
  if (restarting_) {
    stage->setGfxEnabled(stageConsole_->isChecked());
  }
  stageConsole_->setListener(new StageConsoleListener(stage));
  char *cacheDir = getCacheDirCopy();
  try {
    engine_->initStage(stagesBaseDir_, stageName, cacheDir);
    engine_->initShips(shipsBaseDir_, teamNames, numUserTeams, cacheDir);
    teamConsoles_ = guiPrintHandler->getTeamConsoles();

    for (int x = 0; x < engine_->getNumTeams(); x++) {
      teamConsoles_[x]->SetTitle(engine_->getTeam(x)->name);
      teamConsoles_[x]->println();
    }
    stageConsole_->println();
  } catch (EngineException *e) {
#ifdef __WXOSX__
    // Since we initialized the window early.
    delete window_;
    window_ = 0;
#endif
    errorConsole_->print(stageName);
    errorConsole_->print(": ");
    errorConsole_->println(e->what());
    wxMessageDialog errorMessage(NULL, e->what(),
        "BerryBots engine init failed", wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete engine_;
    engine_ = 0;
    delete cacheDir;
    restarting_ = false;
    newMatchDialog_->Show();
    delete e;
    return;
  }
  delete cacheDir;

  viewWidth_ = stage->getWidth() + (STAGE_MARGIN * 2);
  viewHeight_ = stage->getHeight() + (STAGE_MARGIN * 2);
  unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
  unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
  double windowScale;
  if (restarting_ && maintainWindowProperties) {
    windowScale = prevScale;
  } else {
    windowScale =
        std::min(1.0, std::min(((double) screenWidth - DOCK_SIZE) / viewWidth_,
                               ((double) screenHeight) / viewHeight_));
  }
  unsigned int targetWidth = round(windowScale * viewWidth_) + DOCK_SIZE;
  unsigned int targetHeight = round(windowScale * viewHeight_);
#ifdef __WXOSX__
  window->setSize(sf::Vector2u(targetWidth, targetHeight));
  sf::Vector2i pos = window->getPosition();
  int x = limit(0, pos.x, (int) round(screenWidth - targetWidth));
  int y = limit(0, pos.y, (int) round(screenHeight - targetHeight));
  window->setPosition(sf::Vector2i(x, y));
#else
  window = initMainWindow(targetWidth, targetHeight);
#endif

  if (maintainWindowProperties) {
    int left = limit(0, prevPosition.x, screenWidth - targetWidth);
    int top = limit(0, prevPosition.y, screenHeight - targetHeight);
    window_->setPosition(sf::Vector2i(left, top));
  }

  interrupted_ = false;
  paused_ = false;
  newMatchDialog_->Hide();
  packageStageDialog_->Hide();
  packageShipDialog_->Hide();
  gfxHandler_ = new GfxEventHandler();
  stage->addEventHandler((EventHandler*) gfxHandler_);
  stage->addEventHandler(consoleHandler_);

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
  gfxManager_->initViews(window, viewWidth_, viewHeight_);
  window->setVisible(true);
  window->clear();
  gfxManager_->drawGame(window, stage, engine_->getShips(),
                        engine_->getNumShips(), engine_->getGameTime(),
                        gfxHandler_, false, false, engine_->getWinnerName());
  window->display();

  runCurrentMatch();
}

// TODO: Track and display TPS in GUI.
void GuiManager::runCurrentMatch() {
  interrupted_ = false;
  restarting_ = false;
  sf::RenderWindow *window = getMainWindow();
  try {
    while (window->isOpen() && !interrupted_ && !restarting_ && !quitting_) {
      while (!paused_ && !restarting_ && !engine_->isGameOver()
          && engine_->getGameTime() < nextDrawTime_) {
        engine_->processTick();
      }
      
      while (!interrupted_ && !restarting_ && !quitting_
             && (nextDrawTime_ <= engine_->getGameTime()
                 || engine_->isGameOver())) {
        processMainWindowEvents(window, gfxManager_, viewWidth_, viewHeight_);
        clearTeamErroredForActiveConsoles(engine_);
        drawFrame(window);
        if (!paused_ && !engine_->isGameOver()) {
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
    delete e;
    return;
  }

  if (!window->isOpen()) {
    listener_->onAllWindowsClosed();
  }

  // TODO: Display CPU usage in GUI

  if (!interrupted_) {
    gfxManager_->destroyBbGfx();
    delete engine_;
    engine_ = 0;
    delete gfxHandler_;
    gfxHandler_ = 0;
  }
}

void GuiManager::drawFrame(sf::RenderWindow *window) {
  window->clear();
  gfxManager_->drawGame(window, engine_->getStage(), engine_->getShips(),
                        engine_->getNumShips(), engine_->getGameTime(),
                        gfxHandler_, paused_, engine_->isGameOver(),
                        engine_->getWinnerName());
  window->display();
}

void GuiManager::clearTeamErroredForActiveConsoles(BerryBotsEngine *engine) {
  for (int x = 0; x < engine_->getNumTeams(); x++) {
    Team *team = engine_->getTeam(x);
    if (team->errored && teamConsoles_[x]->IsActive()) {
      team->errored = false;
    }
  }
}

void GuiManager::resumeMatch() {
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

void GuiManager::processMainWindowEvents(sf::RenderWindow *window,
    GfxManager *gfxManager, int viewWidth, int viewHeight) {
  sf::Event event;
  bool resized = false;
  while (window->pollEvent(event)) {
#ifdef __WXOSX__
    // For some reason, the dash key (between 0 and =) generates no KeyPressed
    // event on Mac OS X, but does generate a TextEntered event - at least on my
    // MacBook Pro, with built-in or external keyboard. The same external
    // keyboard does generate proper KeyPressed events on Windows and Linux.
    if (event.type == sf::Event::TextEntered
        && (sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::RSystem)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::LSystem))
        && event.text.unicode == 45) {
      gfxManager->decreaseWindowSize(window, viewWidth, viewHeight);
    }
#endif

    if (event.type == sf::Event::Closed) {
      window->close();
      quit();
    }
    if (event.type == sf::Event::MouseWheelMoved) {
      sf::Event::MouseWheelEvent wheelEvent = event.mouseWheel;
      gfxManager->processMouseWheel(wheelEvent.x, wheelEvent.y,
                                    wheelEvent.delta);
    }
    if (event.type == sf::Event::Resized && !resized) {
      resized = true;
      gfxManager->onResize(window, viewWidth, viewHeight);
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
        case sf::Keyboard::Equal:
        case sf::Keyboard::Add:
          if (event.key.system || event.key.control) {
            gfxManager->increaseWindowSize(window, viewWidth, viewHeight);
          }
          break;
        case sf::Keyboard::Dash:
        case sf::Keyboard::Subtract:
          if (event.key.system || event.key.control) {
            gfxManager->decreaseWindowSize(window, viewWidth, viewHeight);
          }
          break;
        case sf::Keyboard::Num0:
          if (event.key.system || event.key.control) {
            gfxManager->defaultWindowSize(window, viewWidth, viewHeight);
          }
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
    // smoother if we use vsync instead of a fixed frame rate, so do that when
    // we have focus.
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
  // this thread has control unless we wxYield each frame. Seems to be
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
            && (event.key.code == sf::Keyboard::Escape
                || event.key.code == sf::Keyboard::Space
                || (event.key.code == sf::Keyboard::W
                    && (event.key.control || event.key.system))))) {
      window->close();
      newMatchDialog_->focusStageSelect();
    }
    if (event.type == sf::Event::Resized && !resized) {
      resized = true;
      gfxManager->onResize(window, viewWidth, viewHeight);
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
  if (previewing_) {
    // Don't allow showStagePreview to execute recursively.
    return;
  }
  previewing_ = true;
  closingPreview_ = false;

#ifdef __WXOSX__
  // On Mac OS X, we need to initialize before the wxWidgets stuff below or we
  // hit some unexplainable crashes when we delete an SFML window. I don't know
  // why, I've merely devised a work-around. Judging from some SFML forum
  // threads, it sounds likely to be an issue with nightmare-ish video drivers.
  initPreviewWindow(600, 600);
#endif

  GfxEventHandler *gfxHandler = new GfxEventHandler();
  BerryBotsEngine *engine = new BerryBotsEngine(fileManager_);
  try {
    engine->initStage(stagesBaseDir_, stageName, getCacheDir().c_str());
  } catch (EngineException *e) {
    delete gfxHandler;
    delete engine;
    errorConsole_->println(e->what());
    wxMessageDialog errorMessage(NULL, e->what(), "Preview failure",
                                 wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete e;
    return;
  }

  Stage *stage = engine->getStage();
  unsigned int viewWidth = stage->getWidth() + (2 * STAGE_MARGIN);
  unsigned int viewHeight = stage->getHeight() + (2 * STAGE_MARGIN);
  unsigned int screenWidth = 600;
  unsigned int screenHeight = 600;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth) / viewWidth,
                             ((double) screenHeight) / viewHeight));
  unsigned int targetWidth = round(windowScale * viewWidth);
  unsigned int targetHeight = round(windowScale * viewHeight);
  wxPoint newMatchPosition = newMatchDialog_->GetPosition();

  char *description = fileManager_->getStageDescription(
      stagesBaseDir_, stageName, getCacheDir().c_str());
  if (!isWhitespace(description)) {
    std::string descTitle("Description: ");
    descTitle.append(stageName);
    previewConsole_->SetTitle(descTitle);
    previewConsole_->clear();
    previewConsole_->print(description);
#ifdef __WINDOWS__
    int spacer = 20;
#else
    int spacer = 5;
#endif
    previewConsole_->SetPosition(
        wxPoint(newMatchPosition.x + targetWidth + 25 + spacer,
                newMatchPosition.y + 25));
    previewConsole_->Show();
    previewConsole_->Raise();
  }

  std::string previewTitle("Preview: ");
  previewTitle.append(stageName);
#ifdef __WXOSX__
  previewWindow_->setSize(sf::Vector2u(targetWidth, targetHeight));
  sf::Vector2i pos = previewWindow_->getPosition();
  int x = limit(0, pos.x, (int) round(screenWidth - targetWidth));
  int y = limit(0, pos.y, (int) round(screenHeight - targetHeight));
  previewWindow_->setPosition(sf::Vector2i(x, y));
#else
  initPreviewWindow(targetWidth, targetHeight);
#endif

#ifdef __WINDOWS__
  previewWindow_->setIcon(32, 32, windowIcon_.getPixelsPtr());
#elif defined(__WXGTK__)
  previewWindow_->setIcon(128, 128, windowIcon_.getPixelsPtr());
#endif

  previewWindow_->setPosition(sf::Vector2i(newMatchPosition.x + 25,
                                           newMatchPosition.y + 25));
  Team **teams = new Team*[1];
  teams[0] = new Team;
  strcpy(teams[0]->name, "PreviewTeam");
  teams[0]->numRectangles = 0;
  teams[0]->numLines = 0;
  teams[0]->numCircles = 0;
  teams[0]->numTexts = 0;
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
  stage->setTeamsAndShips(teams, 1, ships, 1);

  previewGfxManager_->initBbGfx(previewWindow_, viewHeight, stage, teams, 1,
                                ships, 1, resourcePath());
  previewGfxManager_->initViews(previewWindow_, viewWidth, viewHeight);

  while (!quitting_ && !closingPreview_ && previewWindow_->isOpen()) {
    processPreviewWindowEvents(previewWindow_, previewGfxManager_, viewWidth,
                               viewHeight);
    previewWindow_->clear();
    previewGfxManager_->drawGame(previewWindow_, stage, ships, 1, 0, gfxHandler,
                                 false, false, 0);
    previewWindow_->display();
  }

  previewWindow_->close();
  previewGfxManager_->destroyBbGfx();
  delete gfxHandler;
  delete engine;
  delete properties;
  delete teams[0];
  delete teams;

  newMatchDialog_->Show();
  newMatchDialog_->Raise();
  previewing_ = false;
}

void GuiManager::closeStagePreview() {
  closingPreview_ = true;
  previewConsole_->Hide();
}

void GuiManager::launchGameRunner() {
  
}

void GuiManager::deleteStageConsole() {
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

void GuiManager::dialogClosed() {
  if (window_ == 0) {
    listener_->onAllWindowsClosed();
  } else {
    resumeMatch();
  }
}

void GuiManager::dialogEscaped() {
  if (window_ != 0) {
    resumeMatch();
  }
}

void GuiManager::newMatchInitialFocus() {
  newMatchDialog_->focusStageSelect();
}

void GuiManager::packageShipInitialFocus() {
  packageShipDialog_->focusItemSelect();
}

void GuiManager::packageStageInitialFocus() {
  packageStageDialog_->focusItemSelect();
}

void GuiManager::printShipDestroyed(Ship *destroyedShip, Ship *destroyerShip,
                                    int time) {
  std::stringstream timeStream;
  timeStream << " @ " << time;
  std::string timeString = timeStream.str();
  const char *atTime = timeString.c_str();

  if (destroyedShip->index == destroyerShip->index) {
    OutputConsole *console = teamConsoles_[destroyerShip->teamIndex];
    console->print("== ");
    console->print(destroyerShip->properties->name);
    console->print(" destroyed itself");
    console->println(atTime);
  } else {
    OutputConsole *destroyerConsole = teamConsoles_[destroyerShip->teamIndex];
    destroyerConsole->print("== ");
    destroyerConsole->print(destroyerShip->properties->name);
    destroyerConsole->print(" destroyed ");
    destroyerConsole->print((destroyedShip->teamIndex == destroyerShip->teamIndex)
        ? "friendly" : "enemy");
    destroyerConsole->print(" ship: ");
    destroyerConsole->print(destroyedShip->properties->name);
    destroyerConsole->println(atTime);
    
    OutputConsole *destroyeeConsole = teamConsoles_[destroyedShip->teamIndex];
    destroyeeConsole->print("== ");
    destroyeeConsole->print(destroyedShip->properties->name);
    destroyeeConsole->print(" destroyed by: ");
    destroyeeConsole->print(destroyerShip->properties->name);
    destroyeeConsole->println(atTime);
  }
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

void GuiManager::redrawMainWindow() {
  if (window_ != 0) {
    drawFrame(window_);
  }
}

void GuiManager::logErrorMessage(lua_State *L, const char *formatString) {
  const char *luaMessage = lua_tostring(L, -1);
  int messageLen = (int) (strlen(formatString) + strlen(luaMessage) - 2);
  char *errorMessage = new char[messageLen + 1];
  sprintf(errorMessage, formatString, luaMessage);
  errorConsole_->println(errorMessage);
  delete errorMessage;
  errorConsole_->Show();
  errorConsole_->Raise();
}

char* GuiManager::getStagesDirCopy() {
  char *stagesDir = new char[getStagesDir().length() + 1];
  strcpy(stagesDir, getStagesDir().c_str());
  return stagesDir;
}

char* GuiManager::getShipsDirCopy() {
  char *shipsDir = new char[getShipsDir().length() + 1];
  strcpy(shipsDir, getShipsDir().c_str());
  return shipsDir;
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

MatchStarter::MatchStarter(GuiManager *guiManager, char *stagesDir,
                           char *shipsDir) {
  guiManager_ = guiManager;
  stagesDir_ = new char[strlen(stagesDir) + 1];
  strcpy(stagesDir_, stagesDir);
  shipsDir_ = new char[strlen(shipsDir) + 1];
  strcpy(shipsDir_, shipsDir);
}

MatchStarter::~MatchStarter() {
  delete stagesDir_;
  delete shipsDir_;
}

void MatchStarter::startMatch(const char *stageName, char **teamNames,
                              int numTeams) {
  guiManager_->startMatch(stageName, teamNames, numTeams);
}

void MatchStarter::previewStage(const char *stageName) {
  guiManager_->showStagePreview(stageName);
}

void MatchStarter::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadShips();
}

void MatchStarter::onClose() {
  guiManager_->dialogClosed();
}

void MatchStarter::onEscape() {
  guiManager_->dialogEscaped();
}

void MatchStarter::onActive() {
  guiManager_->closeStagePreview();
}

void MatchStarter::onUpdateUi() {
  guiManager_->redrawMainWindow();
}

void MatchStarter::reloadBaseDirs() {
  guiManager_->reloadBaseDirs();
  refreshFiles();
}

ShipPackager::ShipPackager(GuiManager *guiManager, FileManager *fileManager,
                           OutputConsole *packagingConsole, char *shipsDir) {
  packagingConsole_ = packagingConsole;
  guiManager_ = guiManager;
  fileManager_ = fileManager;
  shipsDir_ = new char[strlen(shipsDir) + 1];
  strcpy(shipsDir_, shipsDir);
}

ShipPackager::~ShipPackager() {
  delete shipsDir_;
}

void ShipPackager::package(const char *shipName, const char *version,
                           bool obfuscate) {
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageShip(shipsDir_, shipName, version, cacheDir, tmpDir,
                              obfuscate, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageShip(shipsDir_, shipName, version, cacheDir, tmpDir,
                                obfuscate, true);
      fileManager_->deleteFromCache(cacheDir, e->what());
    } else {
      refresh = false;
    }
    delete e;
  } catch (std::exception *e) {
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging ship failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
    delete e;
  }
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadShips();
  }
}

void ShipPackager::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadShips();
}

void ShipPackager::onClose() {
  guiManager_->dialogClosed();
}

void ShipPackager::onEscape() {
  guiManager_->dialogEscaped();
}

void ShipPackager::onUpdateUi() {
  guiManager_->redrawMainWindow();
}

StagePackager::StagePackager(GuiManager *guiManager, FileManager *fileManager,
    OutputConsole *packagingConsole, char *stagesDir, char *shipsDir) {
  packagingConsole_ = packagingConsole;
  guiManager_ = guiManager;
  fileManager_ = fileManager;
  stagesDir_ = new char[strlen(stagesDir) + 1];
  strcpy(stagesDir_, stagesDir);
  shipsDir_ = new char[strlen(shipsDir) + 1];
  strcpy(shipsDir_, shipsDir);
}

StagePackager::~StagePackager() {
  delete stagesDir_;
  delete shipsDir_;
}

void StagePackager::package(const char *stageName, const char *version,
                            bool obfuscate) {
  char *cacheDir = guiManager_->getCacheDirCopy();
  char *tmpDir = guiManager_->getTmpDirCopy();
  bool refresh = true;
  try {
    fileManager_->packageStage(stagesDir_, stageName, version, cacheDir, tmpDir,
                               obfuscate, false);
  } catch (FileExistsException *e) {
    std::stringstream overwriteStream;
    overwriteStream << "File already exists: " << e->what() << std::endl
                    << std::endl << "OK to overwrite it?";
    wxMessageDialog errorMessage(NULL, overwriteStream.str(), "Are you sure?",
                                 wxOK | wxCANCEL | wxICON_QUESTION);
    int r = errorMessage.ShowModal();
    if (r == wxID_OK) {
      fileManager_->packageStage(stagesDir_, stageName, version, cacheDir,
                                 tmpDir, obfuscate, true);
      fileManager_->deleteFromCache(cacheDir, e->what());
    } else {
      refresh = false;
    }
    delete e;
  } catch (std::exception *e) {
    packagingConsole_->clear();
    packagingConsole_->Show();
    packagingConsole_->println("Packaging stage failed: ");
    packagingConsole_->print("  ");
    packagingConsole_->println(e->what());
    refresh = false;
    delete e;
  }
  delete cacheDir;
  delete tmpDir;
  if (refresh) {
    guiManager_->loadStages();
  }
}

void StagePackager::refreshFiles() {
  guiManager_->loadStages();
  guiManager_->loadShips();
}

void StagePackager::onClose() {
  guiManager_->dialogClosed();
}

void StagePackager::onEscape() {
  guiManager_->dialogEscaped();
}

void StagePackager::onUpdateUi() {
  guiManager_->redrawMainWindow();
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

void PrintStateListener::newTeam(Team *team, const char *filename) {
  guiPrintHandler_->registerTeam(team, filename);
}

ConsoleEventHandler::ConsoleEventHandler(GuiManager *guiManager) {
  guiManager_ = guiManager;
}

void ConsoleEventHandler::handleShipDestroyed(Ship *destroyedShip, int time,
    Ship **destroyerShips, int numDestroyers) {
  for (int x = 0; x < numDestroyers; x++) {
    guiManager_->printShipDestroyed(destroyedShip, destroyerShips[x], time);
  }
}

StageConsoleListener::StageConsoleListener(Stage *stage) {
  stage_ = stage;
}

void StageConsoleListener::onCheck(bool checked) {
  stage_->setGfxEnabled(checked);
}

PreviewConsoleListener::PreviewConsoleListener(GuiManager *guiManager) {
  guiManager_ = guiManager;
}

void PreviewConsoleListener::onClose() {
  guiManager_->closeStagePreview();
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
