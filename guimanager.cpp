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

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include "ResourcePath.hpp"
#include <platformstl/filesystem/readdir_sequence.hpp>
using platformstl::readdir_sequence;

#include "bbutil.h"
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
  stageConsole_ = 0;
  teamConsoles_ = 0;
  numTeams_ = 0;
  gfxManager_ = new GfxManager(true);
  gfxManager_->setListener(new ViewListener(this));
  fileManager_ = new FileManager();
  newMatchDialog_->Show();
  newMatchDialog_->wxWindow::SetFocus();
  stageBaseDir_ = 0;
  botsBaseDir_ = 0;
  paused_ = false;
  consoleId_ = 100;
  matchId_ = 0;
  engine = 0;
}

GuiManager::~GuiManager() {
  clearConsoles();
  delete newMatchDialog_;
  delete packageShipDialog_;
  delete packageStageDialog_;
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
}

void GuiManager::loadStages(const char *baseDir) {
  if (stageBaseDir_ != 0) {
    delete stageBaseDir_;
  }
  stageBaseDir_ = new char[strlen(baseDir) + 1];
  strcpy(stageBaseDir_, baseDir);

  readdir_sequence dir(baseDir, readdir_sequence::files);
  readdir_sequence::const_iterator first = dir.begin();
  readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    readdir_sequence::const_iterator file = first++;
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
  // TODO: load up stage file and try to determine if it really is valid
  return fileManager_->isLuaFilename(stageFilename)
      || fileManager_->isZipFilename(stageFilename);
}

void GuiManager::loadBots(const char *baseDir) {
  if (botsBaseDir_ != 0) {
    delete botsBaseDir_;
  }
  botsBaseDir_ = new char[strlen(baseDir) + 1];
  strcpy(botsBaseDir_, baseDir);

  readdir_sequence dir(baseDir, readdir_sequence::files);
  readdir_sequence::const_iterator first = dir.begin();
  readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    if (isValidBotFile(baseDir, filename)) {
      newMatchDialog_->addBot(filename);
    }
  }
}

bool GuiManager::isValidBotFile(const char *baseDir, char *botFilename) {
  // TODO: load up bot file and try to determine if it really is valid
  return fileManager_->isLuaFilename(botFilename)
      || fileManager_->isZipFilename(botFilename);
}

void GuiManager::linkListeners() {
  newMatchDialog_->setListener(
      new MatchStarter(this, stageBaseDir_, botsBaseDir_));
  packageStageDialog_->setListener(
      new StagePackager(this, fileManager_, stageBaseDir_, botsBaseDir_));
}

void GuiManager::runMatch(char *stageName, char **teamNames, int numTeams) {
  unsigned int thisMatchId = ++matchId_;
  paused_ = false;
  clearConsoles();
  newMatchDialog_->Hide();
  if (printHandler != 0) {
    delete printHandler;
    printHandler = 0;
  }

  srand((unsigned int) time(NULL));
  if (engine != 0) {
    delete engine;
  }
  engine = new BerryBotsEngine();
  stage = engine->getStage();
  const char *cacheDir = getCacheDir().c_str();
  engine->initStage(stageName, cacheDir);
  engine->initShips(teamNames, numTeams, cacheDir);
  GfxEventHandler *gfxHandler = new GfxEventHandler();
  stage->addEventHandler((EventHandler*) gfxHandler);
  
  unsigned int viewWidth = stage->getWidth() + (STAGE_MARGIN * 2);
  unsigned int viewHeight = stage->getHeight() + (STAGE_MARGIN * 2);
  unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
  unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
  double windowScale = min(1.0,
                           min(((double) screenWidth - DOCK_SIZE) / viewWidth,
                               ((double) screenHeight) / viewHeight));
  unsigned int targetWidth = floor(windowScale * viewWidth) + DOCK_SIZE;
  unsigned int targetHeight = floor(windowScale * viewHeight);
  
  window_->setSize(sf::Vector2u(targetWidth, targetHeight));
  gfxManager_->updateView(window_, viewWidth, viewHeight);
  gfxManager_->initBbGfx(window_, viewHeight, stage, engine->getTeams(),
                         engine->getNumTeams(), engine->getShips(),
                         engine->getNumShips(), resourcePath());
  window_->setVisible(true);
  gfxManager_->drawGame(window_, stage, engine->getShips(),
                        engine->getNumShips(), engine->getGameTime(),
                        gfxHandler, false);

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

    if (thisMatchId == matchId_) {
      window_->clear();
      gfxManager_->drawGame(window_, stage, engine->getShips(),
          engine->getNumShips(), engine->getGameTime(), gfxHandler,
          engine->isGameOver());
      window_->display();
      
      time(&realTime2);
      if (realTime2 - realTime1 > 0) {
        realSeconds++;
        if (realSeconds % 10 == 0) {
          // TODO: How to display TPS in GUI
  //        cout << "TPS: " << (((double) engine->getGameTime()) / realSeconds)
  //        << endl;
        }
      }
      realTime1 = realTime2;
    }
  }

  // TODO: how to display all this in GUI
//  char* winnerName = engine->getWinnerName();
//  if (winnerName != 0) {
//    cout << winnerName << " wins! Congratulations!" << endl;
//    delete winnerName;
//  }
//  
//  cout << endl << "CPU time used per tick (microseconds):" << endl;
//  for (int x = 0; x < engine->getNumTeams(); x++) {
//    Team *team = engine->getTeam(x);
//    cout << "  " << team->name << ": "
//    << (team->totalCpuTime / engine->getGameTime()) << endl;
//  }
//  
//  if (realSeconds > 0) {
//    cout << "TPS: " << (((double) engine->getGameTime()) / realSeconds) << endl;
//  }

  gfxManager_->destroyBbGfx();
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

void GuiManager::clearConsoles() {
  if (stageConsole_ != 0) {
    stageConsole_->Hide();
    delete stageConsole_;
    stageConsole_ = 0;
  }
  if (teamConsoles_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      teamConsoles_[x]->Close();
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

void GuiManager::quit() {
  matchId_ = 0;
  gfxManager_->destroyBbGfx();
  delete gfxManager_;
}

MatchStarter::MatchStarter(GuiManager *guiManager, char *stageDir,
                           char *botsDir) {
  guiManager_ = guiManager;
  stageDir_ = new char[strlen(stageDir) + 1];
  strcpy(stageDir_, stageDir);
  botsDir_ = new char[strlen(botsDir) + 1];
  strcpy(botsDir_, botsDir);
}

MatchStarter::~MatchStarter() {
  delete stageDir_;
  delete botsDir_;
}

void MatchStarter::startMatch(const char *stageName, const char **teamNames,
                              int numTeams) {
  char *stagePath = new char[strlen(stageDir_) + strlen(stageName) + 2];
  sprintf(stagePath, "%s/%s", stageDir_, stageName);
  char **teamPaths = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    char *teamPath = new char[strlen(botsDir_) + strlen(teamNames[x]) + 2];
    sprintf(teamPath, "%s/%s", botsDir_, teamNames[x]);
    teamPaths[x] = teamPath;
  }
  guiManager_->runMatch(stagePath, teamPaths, numTeams);

  delete stagePath;
  for (int x = 0; x < numTeams; x++) {
    delete teamPaths[x];
  }
  delete teamPaths;
}

void MatchStarter::cancel() {
  guiManager_->hideNewMatchDialog();
  guiManager_->resumeMatch();
}

StagePackager::StagePackager(GuiManager *guiManager, FileManager *fileManager,
                             char *stageDir, char *botsDir) {
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
  sprintf(stagePath, "%s/%s", stageDir_, stageName);
  fileManager_->packageStage(stagePath, version, getCacheDir().c_str(),
                             getTmpDir().c_str(), nosrc);
  delete stagePath;
}

void StagePackager::cancel() {
  guiManager_->hidePackageStageDialog();
  guiManager_->resumeMatch();
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
