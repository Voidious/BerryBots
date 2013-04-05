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

#include <string.h>
#include <pthread.h>
#include "bbrunner.h"

BerryBotsRunner::BerryBotsRunner(int threadCount) {
  threadCount_ = threadCount;
  threads_ = new pthread_t[threadCount_];
  numMatches_ = 0;
}

BerryBotsRunner::~BerryBotsRunner() {
  delete threads_;
  for (int x = 0; x < numMatches_; x++) {
    delete matches_[x];
  }
}

void BerryBotsRunner::queueMatch(const char *stageName, char **shipNames,
                                 int numShips) {
  MatchConfig *matchConfig = new MatchConfig(stageName, shipNames, numShips);
  matches_[numMatches_++] = matchConfig;
}

MatchConfig::MatchConfig(const char *stageName, char **shipNames,
                         int numShips) {
  stageName_ = new char[strlen(stageName) + 1];
  strcpy(stageName_, stageName);
  shipNames_ = new char*[numShips];
  for (int x = 0; x < numShips; x++) {
    shipNames_[x] = new char[strlen(shipNames[x]) + 1];
    strcpy(shipNames_[x], shipNames[x]);
  }
  numShips_ = numShips;
  winnerName_ = 0;
  processedResult_ = false;
}

MatchConfig::~MatchConfig() {
  delete stageName_;
  for (int x = 0; x < numShips_; x++) {
    delete shipNames_[x];
  }
  delete shipNames_;
  if (winnerName_ != 0) {
    delete winnerName_;
  }
}

const char* MatchConfig::getStageName() {
  return stageName_;
}

char** MatchConfig::getShipNames() {
  return shipNames_;
}

int MatchConfig::getNumShips() {
  return numShips_;
}

const char* MatchConfig::getWinnerName() {
  return winnerName_;
}

void MatchConfig::setWinnerName(const char *name) {
  winnerName_ = new char[strlen(name) + 1];
  strcpy(winnerName_, name);
}

bool MatchConfig::getProcessedResult() {
  return processedResult_;
}

void MatchConfig::setProcessedResult() {
  processedResult_ = true;
}
