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

#include <iostream>
#include <exception>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "shapes.h"
#include "bbconst.h"
#include "bbutil.h"
#include "bbpigfx.h"
#include "filemanager.h"
#include "stage.h"
#include "bbengine.h"
#include "gfxeventhandler.h"
#include "printhandler.h"
#include "cliprinthandler.h"
#include "clipackagereporter.h"
#include "tarzipper.h"

BerryBotsEngine *engine = 0;
Stage *stage = 0;
PrintHandler *printHandler = 0;

void printUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << "  berrybots.sh [-nodisplay] <stage.lua> <bot1.lua> [<bot2.lua> ...]"
            << std::endl;
  std::cout << "  OR" << std::endl;
  std::cout << "  berrybots.sh [-nosrc] -packstage <stage.lua> <version>"
            << std::endl;
  std::cout << "  OR" << std::endl;
  std::cout << "  berrybots.sh [-nosrc] -packbot <bot.lua> <version>"
            << std::endl;
  exit(0);
}

int main(int argc, char *argv[]) {
  Zipper *zipper = new TarZipper();
  FileManager *fileManager = new FileManager(zipper);
  CliPackageReporter *packageReporter = new CliPackageReporter();
  fileManager->setListener(packageReporter);

  char *botsBaseDir = FileManager::getAbsFilePath(BOTS_SUBDIR);
  char *stagesBaseDir = FileManager::getAbsFilePath(STAGES_SUBDIR);

  char **stageInfo = parseFlag(argc, argv, "packstage", 2);
  if (stageInfo != 0) {
    if (argc < 4) {
      printUsage();
    }
    bool nosrc = flagExists(argc, argv, "nosrc");
    try {
      char *stageAbsName = FileManager::getAbsFilePath(stageInfo[0]);
      char *stageName =
          FileManager::parseRelativeFilePath(stagesBaseDir, stageAbsName);
      fileManager->packageStage(stagesBaseDir, stageName, stageInfo[1],
                                CACHE_SUBDIR, TMP_SUBDIR, nosrc, true);
      delete stageAbsName;
      delete stageName;
    } catch (std::exception *e) {
      std::cout << "BerryBots encountered an error:" << std::endl;
      std::cout << "  " << e->what() << std::endl;
      exit(0);
    }
    delete stageInfo;
    return 0;
  }

  char **botInfo = parseFlag(argc, argv, "packbot", 2);
  if (botInfo != 0) {
    if (argc < 4) {
      printUsage();
    }
    bool nosrc = flagExists(argc, argv, "nosrc");
    try {
      char *botAbsName = FileManager::getAbsFilePath(botInfo[0]);
      char *botName =
         FileManager::parseRelativeFilePath(botsBaseDir, botAbsName);
      fileManager->packageBot(botsBaseDir, botName, botInfo[1], CACHE_SUBDIR,
                              TMP_SUBDIR, nosrc, true);
      delete botAbsName;
      delete botName;
    } catch (std::exception *e) {
      std::cout << "BerryBots encountered an error:" << std::endl;
      std::cout << "  " << e->what() << std::endl;
      exit(0);
    }
    delete botInfo;
    return 0;
  }

  bool nodisplay = flagExists(argc, argv, "nodisplay");

  if (argc < 3 || (nodisplay && strcmp(argv[1], "-nodisplay") != 0)) {
    printUsage();
  }

  srand(time(NULL));
  int screenWidth;
  int screenHeight;
  init(&screenWidth, &screenHeight);

  engine = new BerryBotsEngine(fileManager);
  stage = engine->getStage();

  char *stageAbsName = FileManager::getAbsFilePath(argv[nodisplay ? 2 : 1]);
  char *stageName =
      FileManager::parseRelativeFilePath(stagesBaseDir, stageAbsName);
  try {
    engine->initStage(stagesBaseDir, stageName, CACHE_SUBDIR);
  } catch (EngineException *e) {
    delete stageAbsName;
    delete stageName;
    std::cout << "BerryBots initialization failed:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    exit(0);
  }
  delete stageAbsName;
  delete stageName;

  int firstTeam = (nodisplay ? 3 : 2);
  int numTeams = argc - firstTeam;
  char **teams = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    char *teamAbsName = FileManager::getAbsFilePath(argv[x + firstTeam]);
    char *teamName =
        FileManager::parseRelativeFilePath(botsBaseDir, teamAbsName);
    teams[x] = teamName;
    delete teamAbsName;
  }

  try {
    engine->initShips(botsBaseDir, teams, numTeams, CACHE_SUBDIR);
  } catch (EngineException *e) {
    std::cout << "BerryBots initialization failed:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    exit(0);
  }

  printHandler = new CliPrintHandler(engine->getTeams(), numTeams);

  GfxEventHandler *gfxHandler = 0;
  if (!nodisplay) {
    gfxHandler = new GfxEventHandler();
    stage->addEventHandler((EventHandler*) gfxHandler);
    initVgGfx(screenWidth, screenHeight, stage, engine->getShips(),
        engine->getNumShips());
    drawGame(screenWidth, screenHeight, stage, engine->getShips(),
        engine->getNumShips(), engine->getGameTime(), gfxHandler);
  }

  time_t realTime1;
  time_t realTime2;
  time(&realTime1);
  int realSeconds = 0;

  try {
    do {
      engine->processTick();
      if (!nodisplay) {
        drawGame(screenWidth, screenHeight, stage, engine->getShips(),
            engine->getNumShips(), engine->getGameTime(), gfxHandler);
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
    } while (!engine->isGameOver());
  } catch (EngineException *e) {
    std::cout << "BerryBots encountered an error:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    exit(0);
  }

  if (!nodisplay) {
    destroyVgGfx();
    finish();
  }

  char* winnerName = engine->getWinnerName();
  if (winnerName != 0) {
    std::cout << winnerName << " wins! Congratulations!" << std::endl;
    delete winnerName;
  }

  std::cout << std::endl << "CPU time used per tick (microseconds):"
            << std::endl;
  for (int x = 0; x < engine->getNumTeams(); x++) {
    Team *team = engine->getTeam(x);
    if (!team->stageShip && !team->doa) {
      std::cout << "  " << team->name << ": "
                << (team->totalCpuTime / team->totalCpuTicks) << std::endl;
    }
  }

  if (realSeconds > 0) {
    std::cout << std::endl << "TPS: "
              << (((double) engine->getGameTime()) / realSeconds) << std::endl;
  }

  delete engine;
  delete teams;
  delete printHandler;
  delete packageReporter;
  delete fileManager;
  delete zipper;
  delete botsBaseDir;
  delete stagesBaseDir;

  return 0;
}
