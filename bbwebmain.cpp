/*
  Copyright (C) 2013 - Voidious

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
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sstream>
#include <math.h>
#include "bbutil.h"
#include "stage.h"
#include "bbengine.h"
#include "replaybuilder.h"
#include "filemanager.h"
#include "cliprinthandler.h"
#include "tarzipper.h"
#include "ResourcePath.hpp"

void printUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << "  ./berrybots <stage.lua> <bot1.lua> [<bot2.lua> ...]" << std::endl;
  std::cout << "Output:" << std::endl;
  std::cout << "  Path to replay file.";
  exit(0);
}

char* replayFilename(const char *stageName) {
  std::stringstream nameStream;
  char *timestamp = getTimestamp();
  nameStream << ((stageName == 0) ? "unknown" : stageName) << "-" << timestamp
             << "-";
  nameStream << std::hex << (rand() % 4096);
  nameStream << ".html";
  delete timestamp;

  std::string filename = nameStream.str();
  char *newFilename = new char[filename.length() + 1];
  strcpy(newFilename, filename.c_str());
  
  return newFilename;
}

int main(int argc, char *argv[]) {
  Zipper *zipper = new TarZipper();
  FileManager *fileManager = new FileManager(zipper);

  char *shipsBaseDir = fileManager->getAbsFilePath(SHIPS_SUBDIR);
  char *stagesBaseDir = fileManager->getAbsFilePath(STAGES_SUBDIR);

  if (argc < 3) {
    printUsage();
  }

  srand(time(NULL));
  CliPrintHandler *printHandler = new CliPrintHandler(true);
  BerryBotsEngine *engine =
      new BerryBotsEngine(printHandler, fileManager, resourcePath().c_str());
  Stage *stage = engine->getStage();

  char *stageAbsName = fileManager->getAbsFilePath(argv[1]);
  char *stageName =
      fileManager->parseRelativeFilePath(stagesBaseDir, stageAbsName);
  if (stageName == 0) {
    std::cout << "Stage must be located under " << STAGES_SUBDIR
              << "/ subdirectory: " << argv[1] << std::endl;
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

  int firstTeam = 2;
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

  printHandler->setNumTeams(numTeams);
  try {
    engine->initShips(shipsBaseDir, teams, numTeams, CACHE_SUBDIR);
  } catch (EngineException *e) {
    std::cout << "BerryBots initialization failed:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    delete e;
    return 0;
  }
  printHandler->updateTeams(engine->getTeams());
  
  try {
    while (!engine->isGameOver() && engine->getGameTime() < 100000) {
      engine->processTick();
    }
  } catch (EngineException *e) {
    std::cout << "BerryBots encountered an error:" << std::endl;
    std::cout << "  " << e->what() << std::endl;
    delete e;
    return 0;
  }
  
  ReplayBuilder *replayBuilder = engine->getReplayBuilder();

  // TODO: move this into a function in the engine
  Team **rankedTeams = engine->getRankedTeams();
  replayBuilder->setResults(rankedTeams, engine->getNumTeams());

  char *filename = 0;
  char *absFilename = 0;
  do {
    if (filename != 0) {
      delete filename;
    }
    if (absFilename != 0) {
      delete absFilename;
    }
    filename = replayFilename(stage->getName());
    char *filePath = fileManager->getFilePath(REPLAYS_SUBDIR, filename);
    absFilename = fileManager->getAbsFilePath(filePath);
    delete filePath;
  } while (fileManager->fileExists(absFilename));
  replayBuilder->saveReplay(filename);
  std::cout << std::endl << "Saved replay to: " << REPLAYS_SUBDIR << "/"
            << filename << std::endl;
  delete filename;
  delete absFilename;

  std::cout << std::endl;

  delete engine;
  for (int x = 0; x < numTeams; x++) {
    delete teams[x];
  }
  delete teams;
  delete rankedTeams;
  delete printHandler;
  delete fileManager;
  delete zipper;
  delete shipsBaseDir;
  delete stagesBaseDir;

  return 0;
}
