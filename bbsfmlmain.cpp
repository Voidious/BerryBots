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

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

#include <iostream>
#include <exception>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "bbutil.h"
#include "stage.h"
#include "bbengine.h"
#include "gfxeventhandler.h"
#include "gfxmanager.h"
#include "filemanager.h"
#include "printhandler.h"
#include "cliprinthandler.h"
#include "clipackagereporter.h"
#include "guizipper.h"

BerryBotsEngine *engine = 0;
Stage *stage = 0;
PrintHandler *printHandler = 0;

void printUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << "  berrybots.sh [-nodisplay] <stage.lua> <bot1.lua> [<bot2.lua> ...]"
            << std::endl;
  std::cout << "  OR" << std::endl;
  std::cout << "  berrybots.sh -packstage <stage.lua> <version>"
            << std::endl;
  std::cout << "  OR" << std::endl;
  std::cout << "  berrybots.sh -packbot <bot.lua> <version>"
            << std::endl;
  exit(0);
}

int main(int argc, char *argv[]) {
  Zipper *zipper = new GuiZipper();
  FileManager *fileManager = new FileManager(zipper);
  CliPackageReporter *packageReporter = new CliPackageReporter();
  fileManager->setListener(packageReporter);

  char *shipsBaseDir = fileManager->getAbsFilePath(SHIPS_SUBDIR);
  char *stagesBaseDir = fileManager->getAbsFilePath(STAGES_SUBDIR);

  if (flagExists(argc, argv, "packstage")) {
    char **stageInfo = parseFlag(argc, argv, "packstage", 2);
    if (stageInfo == 0) {
      printUsage();
    } else {
      // TODO: add a new flag for obfuscating source code
      bool obfuscate = false;
      try {
        char *stageAbsName = fileManager->getAbsFilePath(stageInfo[0]);
        char *stageName =
            fileManager->parseRelativeFilePath(stagesBaseDir, stageAbsName);
        if (stageName == 0) {
          std::cout << "Stage must be located under " << STAGES_SUBDIR
                    << "/ subdirectory: " << stageInfo[0] << std::endl;
        } else {
          fileManager->packageStage(stagesBaseDir, stageName, stageInfo[1],
                                    CACHE_SUBDIR, TMP_SUBDIR, obfuscate, true);
          delete stageName;
        }
        delete stageAbsName;
      } catch (std::exception *e) {
        std::cout << "BerryBots encountered an error:" << std::endl;
        std::cout << "  " << e->what() << std::endl;
        delete e;
      }
      delete stageInfo;
    }
    return 0;
  }

  if (flagExists(argc, argv, "packbot")) {
    char **shipInfo = parseFlag(argc, argv, "packbot", 2);
    if (shipInfo == 0) {
      printUsage();
    } else {
      // TODO: add a new flag for obfuscating source code
      bool obfuscate = false;
      try {
        char *shipAbsName = fileManager->getAbsFilePath(shipInfo[0]);
        char *shipName =
           fileManager->parseRelativeFilePath(shipsBaseDir, shipAbsName);
        if (shipName == 0) {
          std::cout << "Ship must be located under " << SHIPS_SUBDIR
                    << "/ subdirectory: " << shipInfo[0] << std::endl;
        } else {
          fileManager->packageShip(shipsBaseDir, shipName, shipInfo[1],
                                   CACHE_SUBDIR, TMP_SUBDIR, obfuscate, true);
          delete shipName;
        }
        delete shipAbsName;
      } catch (std::exception *e) {
        std::cout << "BerryBots encountered an error:" << std::endl;
        std::cout << "  " << e->what() << std::endl;
        delete e;
      }
      delete shipInfo;
    }
    return 0;
  }
  
  bool nodisplay = flagExists(argc, argv, "nodisplay");
  if (argc < 3 || (nodisplay && strcmp(argv[1], "-nodisplay") != 0)) {
    printUsage();
  }

  srand(time(NULL));
  engine = new BerryBotsEngine(fileManager);
  stage = engine->getStage();

  CliPrintHandler *cliPrintHandler = new CliPrintHandler();
  printHandler = (PrintHandler*) cliPrintHandler;
  CliStateListener *cliStateListener = new CliStateListener(cliPrintHandler);

  char *stageAbsName = fileManager->getAbsFilePath(argv[nodisplay ? 2 : 1]);
  char *stageName =
      fileManager->parseRelativeFilePath(stagesBaseDir, stageAbsName);
  if (stageName == 0) {
    std::cout << "Stage must be located under " << STAGES_SUBDIR
              << "/ subdirectory: " << argv[nodisplay ? 2 : 1] << std::endl;
    return 0;
  }
  try {
    engine->initStage(stagesBaseDir, stageName, CACHE_SUBDIR);
  } catch (EngineException *e) {
    delete stageAbsName;
    delete stageName;
    std::cout << "BerryBots initialization failed:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    delete e;
    return 0;
  }
  delete stageAbsName;
  delete stageName;

  int firstTeam = (nodisplay ? 3 : 2);
  int numTeams = argc - firstTeam;
  char **teams = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    char *teamAbsName = fileManager->getAbsFilePath(argv[x + firstTeam]);
    char *teamName =
        fileManager->parseRelativeFilePath(shipsBaseDir, teamAbsName);
    if (teamName == 0) {
      std::cout << "Ship must be located under " << SHIPS_SUBDIR
                << "/ subdirectory: " << argv[x + firstTeam] << std::endl;
      return 0;
    }
    teams[x] = teamName;
    delete teamAbsName;
  }

  cliPrintHandler->setNumTeams(numTeams);
  engine->setListener(cliStateListener);
  try {
    engine->initShips(shipsBaseDir, teams, numTeams, CACHE_SUBDIR);
  } catch (EngineException *e) {
    std::cout << "BerryBots initialization failed:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    delete e;
    return 0;
  }
  cliPrintHandler->updateTeams(engine->getTeams());

  GfxManager *gfxManager;
  sf::RenderWindow *window = 0;
  GfxEventHandler *gfxHandler = 0;
  unsigned int viewWidth = stage->getWidth() + (STAGE_MARGIN * 2);
  unsigned int viewHeight = stage->getHeight() + (STAGE_MARGIN * 2);
  if (!nodisplay) {
    gfxHandler = new GfxEventHandler();
    stage->addEventHandler((EventHandler*) gfxHandler);

    unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
    unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
    double windowScale = std::min(1.0,
        std::min(((double) screenWidth) / viewWidth,
                 ((double) screenHeight) / viewHeight));
    unsigned int targetWidth = round(windowScale * viewWidth);
    unsigned int targetHeight = round(windowScale * viewHeight);

    gfxManager = new GfxManager(false);
    window = new sf::RenderWindow(sf::VideoMode(targetWidth, targetHeight),
        "BerryBots", sf::Style::Default, sf::ContextSettings(0, 0, 16, 2, 0));
    gfxManager->initViews(window, viewWidth, viewHeight);
    gfxManager->initBbGfx(window, viewHeight, stage, engine->getTeams(),
        engine->getNumTeams(), engine->getShips(), engine->getNumShips(),
        std::string(".") + std::string(BB_DIRSEP));
    window->clear();
    gfxManager->drawGame(window, stage, engine->getShips(),
        engine->getNumShips(), engine->getGameTime(), gfxHandler, false, false,
        0);
    window->display();
  }
  
  time_t realTime1;
  time_t realTime2;
  time(&realTime1);
  int realSeconds = 0;
  
  try {
    while ((nodisplay || window->isOpen()) && !engine->isGameOver()) {
      engine->processTick();
  
      if (!nodisplay) {
        sf::Event event;
        bool resized = false;
        while (window->pollEvent(event)) {
          if (event.type == sf::Event::Closed) {
            window->close();
          }
          if (event.type == sf::Event::KeyPressed
              && event.key.code == sf::Keyboard::Escape) {
            window->close();
          }
          if (event.type == sf::Event::Resized && !resized) {
            resized = true;
            gfxManager->onResize(window, viewWidth, viewHeight);
          }
        }
    
        window->clear();
        gfxManager->drawGame(window, stage, engine->getShips(),
            engine->getNumShips(), engine->getGameTime(), gfxHandler, false,
            false, 0);
        window->display();
      }
  
      time(&realTime2);
      if (realTime2 - realTime1 > 0) {
        realSeconds++;
        if (realSeconds % 10 == 0) {
          std::cout << "TPS: "
                    << (((double) engine->getGameTime()) / realSeconds)
                    << std::endl;
        }
      }
      realTime1 = realTime2;
    }
  } catch (EngineException *e) {
    std::cout << "BerryBots encountered an error:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    delete e;
    return 0;
  }
  
  if (!nodisplay) {
    gfxManager->destroyBbGfx();
    delete window;
  }
  
  const char* winnerName = engine->getWinnerName();
  if (winnerName != 0) {
    std::cout << std::endl<< winnerName << " wins! Congratulations!"
              << std::endl;
  }

  std::cout << std::endl << "Results:" << std::endl;
  Team **rankedTeams = engine->getRankedTeams();
  bool hasScores = false;
  for (int x = 0; x < numTeams; x++) {
    if (rankedTeams[x]->result.score != 0) {
      hasScores = true;
      break;
    }
  }
  TeamResult *firstResult = &(rankedTeams[0]->result);
  int numStats = firstResult->numStats;
  char **statKeys = 0;
  if (numStats > 0) {
    statKeys = new char*[firstResult->numStats];
    for (int x = 0; x < numStats; x++) {
      statKeys[x] = new char[strlen(firstResult->stats[x]->key) + 1];
      strcpy(statKeys[x], firstResult->stats[x]->key);
    }
  }

  for (int x = 0; x < engine->getNumTeams(); x++) {
    TeamResult *result = &(rankedTeams[x]->result);
    if (result->showResult) {
      std::cout << "    " << rankedTeams[x]->name << ":" << std::endl;
      std::cout << "        Rank: ";
      if (result->rank == 0) {
        std::cout << "-";
      } else {
        std::cout << result->rank;
      }
      std::cout << std::endl;
      if (hasScores) {
        std::cout << "        Score: " << round(result->score, 2) << std::endl;
      }

      for (int y = 0; y < numStats; y++) {
        char *key = statKeys[y];
        bool found = false;
        for (int z = 0; z < result->numStats; z++) {
        char *resultKey = result->stats[z]->key;
        if (strcmp(key, resultKey) == 0) {
          std::cout << "        " << key << ": "
              << round(result->stats[z]->value, 2) << std::endl;
          found = true;
          break;
        }
        }
        if (!found) {
        std::cout << "        " << key << ": -" << std::endl;
        }
      }
    }
  }

  std::cout << std::endl << "CPU time used per tick (microseconds):"
            << std::endl;
  for (int x = 0; x < engine->getNumTeams(); x++) {
    Team *team = engine->getTeam(x);
    if (!team->stageShip && !team->disabled) {
      std::cout << "  " << team->name << ": "
                << (team->totalCpuTime / team->totalCpuTicks) << std::endl;
    }
  }

  if (realSeconds > 0) {
    std::cout << std::endl << "TPS: "
              << (((double) engine->getGameTime()) / realSeconds) << std::endl;
  }
  
  delete engine;
  for (int x = 0; x < numTeams; x++) {
    delete teams[x];
  }
  delete teams;
  delete rankedTeams;
  for (int x = 0; x < numStats; x++) {
    delete statKeys[x];
  }
  delete statKeys;
  delete printHandler;
  if (!nodisplay) {
    delete gfxManager;
  }
  delete packageReporter;
  delete fileManager;
  delete zipper;
  delete shipsBaseDir;
  delete stagesBaseDir;

  return 0;
}
