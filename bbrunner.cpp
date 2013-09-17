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
#include <stdlib.h>
#include <platformstl/synch/sleep_functions.h>
#include "basedir.h"
#include "filemanager.h"
#include "zipper.h"
#include "bbengine.h"
#include "replaybuilder.h"
#include "bbrunner.h"

BerryBotsRunner::BerryBotsRunner(int threadCount, Zipper *zipper,
                                 const char *replayTemplateDir) {
  stagesDir_ = new char[getStagesDir().length() + 1];
  strcpy(stagesDir_, getStagesDir().c_str());
  shipsDir_ = new char[getShipsDir().length() + 1];
  strcpy(shipsDir_, getShipsDir().c_str());
  cacheDir_ = new char[getCacheDir().length() + 1];
  strcpy(cacheDir_, getCacheDir().c_str());
  if (replayTemplateDir == 0) {
    replayTemplateDir_ = 0;
  } else {
    replayTemplateDir_ = new char[strlen(replayTemplateDir) + 1];
    strcpy(replayTemplateDir_, replayTemplateDir);
  }
  listener_ = 0;

  schedulerSettings_ = new SchedulerSettings;
  schedulerSettings_->numMatches = 0;
  schedulerSettings_->numThreads = threadCount;
  schedulerSettings_->matchesRunning = 0;
  schedulerSettings_->zipper = zipper;
  schedulerSettings_->done = false;
  schedulerSettings_->randomSeed = rand();
  pthread_create(&schedulerThread_, 0, BerryBotsRunner::scheduler,
                 (void*) schedulerSettings_);
  pthread_detach(schedulerThread_);
}

BerryBotsRunner::~BerryBotsRunner() {
  delete stagesDir_;
  delete shipsDir_;
  delete cacheDir_;
  if (replayTemplateDir_ != 0) {
    delete replayTemplateDir_;
  }
  if (listener_ != 0) {
    delete listener_;
  }
}

void BerryBotsRunner::queueMatch(const char *stageName, char **teamNames,
                                 int numTeams) {
  if (schedulerSettings_->numMatches < MAX_MATCHES) {
    MatchConfig *matchConfig = new MatchConfig(stageName, teamNames, numTeams,
        stagesDir_, shipsDir_, cacheDir_, replayTemplateDir_);
    schedulerSettings_->matches[schedulerSettings_->numMatches++] = matchConfig;
    // TODO: error handling for scheduling > max matches
  }
}

MatchResult* BerryBotsRunner::nextResult() {
  if (allResultsProcessed()) {
    return 0;
  } else {
    while (!schedulerSettings_->done) {
      for (int x = 0; x < schedulerSettings_->numMatches; x++) {
        MatchConfig *config = schedulerSettings_->matches[x];
        if (config->isFinished() && !config->hasProcessedResult()) {
          MatchResult *nextResult = new MatchResult(config->getStageName(),
              config->getTeamNames(), config->getNumTeams(),
              config->getWinnerFilename(), config->getTeamResults(),
              config->hasScores(), config->getReplayBuilder(),
              config->getErrorMessage());
          config->processedResult();
          return nextResult;
        }
      }
      if (listener_ != 0) {
        listener_->refresh();
      }
      platformstl::micro_sleep(SLEEP_INTERVAL);
    }
  }
  return 0;
}

bool BerryBotsRunner::allResultsProcessed() {
  for (int x = 0; x < schedulerSettings_->numMatches; x++) {
    if (!schedulerSettings_->matches[x]->hasProcessedResult()) {
      return false;
    }
  }
  return true;
}

void BerryBotsRunner::quit() {
  schedulerSettings_->done = true;
}

void BerryBotsRunner::setListener(RefresherListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

void *BerryBotsRunner::scheduler(void *vargs) {
  SchedulerSettings *settings = (SchedulerSettings *) vargs;
  srand(settings->randomSeed);
  while (!settings->done) {
    platformstl::micro_sleep(SLEEP_INTERVAL);
    if (settings->matchesRunning < settings->numThreads) {
      MatchConfig *nextMatch = 0;
      for (int x = 0; x < settings->numMatches; x++) {
        MatchConfig *config = settings->matches[x];
        if (!config->isStarted()) {
          config->started();
          nextMatch = config;
          break;
        }
      }
      if (nextMatch != 0) {
        settings->matchesRunning++;
        pthread_t gameThread;
        MatchSettings *matchSettings = new MatchSettings;
        matchSettings->schedulerSettings = settings;
        matchSettings->matchConfig = nextMatch;
        matchSettings->randomSeed = rand();
        pthread_create(&gameThread, 0, BerryBotsRunner::runMatch,
                       (void*) matchSettings);
        pthread_detach(gameThread);
      }
    }
  }

  bool pendingMatches = false;
  do {
    for (int x = 0; x < settings->numMatches; x++) {
      MatchConfig *config = settings->matches[x];
      if (config->isStarted() && !config->isFinished()) {
        pendingMatches = true;
        break;
      }
      if (pendingMatches) {
        platformstl::micro_sleep(SLEEP_INTERVAL);
      }
    }
  } while (pendingMatches);

  for (int x = 0; x < settings->numMatches; x++) {
    delete settings->matches[x];
  }

  delete settings;
  return 0;
}

void* BerryBotsRunner::runMatch(void *vargs) {
  MatchSettings *settings = (MatchSettings *) vargs;
  srand(settings->randomSeed);
  MatchConfig *config = settings->matchConfig;
  SchedulerSettings *schedulerSettings = settings->schedulerSettings;

  FileManager *fileManager = new FileManager(schedulerSettings->zipper);
  BerryBotsEngine *engine =
      new BerryBotsEngine(0, fileManager, config->getReplayTemplateDir());
  bool aborted = false;
  try {
    engine->initStage(config->getStagesDir(), config->getStageName(),
                      config->getCacheDir());
    engine->initShips(config->getShipsDir(), config->getTeamNames(),
                      config->getNumTeams(), config->getCacheDir());
  } catch (EngineException *e) {
    config->setErrorMessage(e->what());
    aborted = true;
    delete e;
  }

  try {
    while (!aborted && !schedulerSettings->done && !engine->isGameOver()) {
      engine->processTick();
    }
  } catch (EngineException *e) {
    config->setErrorMessage(e->what());
    aborted = true;
    delete e;
  }
  if (!aborted && engine->isGameOver()) {
    const char *winner = engine->getWinnerFilename();
    if (winner != 0) {
      config->setWinnerFilename(winner);
    }
    config->setTeamResults(engine->getTeamResults());

    // TODO: move this into a function in the engine
    Team **rankedTeams = engine->getRankedTeams();
    engine->getReplayBuilder()->setResults(rankedTeams, engine->getNumTeams());
    delete rankedTeams;
  }
  config->setHasScores(engine->hasScores());
  config->setReplayBuilder(engine->getReplayBuilder());
  config->finished();
  delete engine;
  delete fileManager;

  schedulerSettings->matchesRunning--;
  delete settings;
  return 0;
}

MatchConfig::MatchConfig(const char *stageName, char **teamNames,
    int numTeams, const char *stagesDir, const char *shipsDir,
    const char *cacheDir, const char *replayTemplateDir) {
  stagesDir_ = new char[strlen(stagesDir) + 1];
  strcpy(stagesDir_, stagesDir);
  shipsDir_ = new char[strlen(shipsDir) + 1];
  strcpy(shipsDir_, shipsDir);
  cacheDir_ = new char[strlen(cacheDir) + 1];
  strcpy(cacheDir_, cacheDir);
  if (replayTemplateDir == 0) {
    replayTemplateDir_ = 0;
  } else {
    replayTemplateDir_ = new char[strlen(replayTemplateDir) + 1];
    strcpy(replayTemplateDir_, replayTemplateDir);
  }

  stageName_ = new char[strlen(stageName) + 1];
  strcpy(stageName_, stageName);
  teamNames_ = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    teamNames_[x] = new char[strlen(teamNames[x]) + 1];
    strcpy(teamNames_[x], teamNames[x]);
  }
  numTeams_ = numTeams;
  winnerFilename_ = 0;
  started_ = finished_ = processedResult_ = hasScores_ = false;
  teamResults_ = 0;
  errorMessage_ = 0;
  replayBuilder_ = 0;
}

MatchConfig::~MatchConfig() {
  delete stageName_;
  for (int x = 0; x < numTeams_; x++) {
    delete teamNames_[x];
  }
  delete teamNames_;
  if (winnerFilename_ != 0) {
    delete winnerFilename_;
  }
  if (teamResults_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      TeamResult *result = teamResults_[x];
      for (int y = 0; y < result->numStats; y++) {
        delete result->stats[y]->key;
        delete result->stats[y];
      }
      delete result;
    }
    delete teamResults_;
  }
  delete stagesDir_;
  delete shipsDir_;
  delete cacheDir_;
  if (replayTemplateDir_ != 0) {
    delete replayTemplateDir_;
  }
  if (errorMessage_ != 0) {
    delete errorMessage_;
  }
}

const char* MatchConfig::getStagesDir() {
  return stagesDir_;
}

const char* MatchConfig::getShipsDir() {
  return shipsDir_;
}

const char* MatchConfig::getCacheDir() {
  return cacheDir_;
}

const char* MatchConfig::getReplayTemplateDir() {
  return replayTemplateDir_;
}

const char* MatchConfig::getStageName() {
  return stageName_;
}

char** MatchConfig::getTeamNames() {
  return teamNames_;
}

int MatchConfig::getNumTeams() {
  return numTeams_;
}

const char* MatchConfig::getWinnerFilename() {
  return winnerFilename_;
}

void MatchConfig::setWinnerFilename(const char *name) {
  winnerFilename_ = new char[strlen(name) + 1];
  strcpy(winnerFilename_, name);
}

TeamResult** MatchConfig::getTeamResults() {
  return teamResults_;
}

void MatchConfig::setTeamResults(TeamResult **teamResults) {
  teamResults_ = teamResults;
}

void MatchConfig::setHasScores(bool hasScores) {
  hasScores_ = hasScores;
}

bool MatchConfig::hasScores() {
  return hasScores_;
}

void MatchConfig::setReplayBuilder(ReplayBuilder *replayBuilder) {
  replayBuilder_ = replayBuilder;
}

ReplayBuilder* MatchConfig::getReplayBuilder() {
  return replayBuilder_;
}

bool MatchConfig::isStarted() {
  return started_;
}

void MatchConfig::started() {
  started_ = true;
}

bool MatchConfig::isFinished() {
  return finished_;
}

void MatchConfig::finished() {
  finished_ = true;
}

bool MatchConfig::hasProcessedResult() {
  return processedResult_;
}

void MatchConfig::processedResult() {
  processedResult_ = true;
}

const char* MatchConfig::getErrorMessage() {
  return errorMessage_;
}

void MatchConfig::setErrorMessage(const char *errorMessage) {
  if (errorMessage_ != 0) {
    delete errorMessage_;
  }
  errorMessage_ = new char[strlen(errorMessage) + 1];
  strcpy(errorMessage_, errorMessage);
}

MatchResult::MatchResult(const char *stageName, char **teamNames, int numTeams,
    const char *winner, TeamResult **teamResults, bool hasScores,
    ReplayBuilder *replayBuilder, const char *errorMessage) {
  stageName_ = new char[strlen(stageName) + 1];
  strcpy(stageName_, stageName);
  teamNames_ = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    teamNames_[x] = new char[strlen(teamNames[x]) + 1];
    strcpy(teamNames_[x], teamNames[x]);
  }
  numTeams_ = numTeams;
  if (winner == 0) {
    winner_ = 0;
  } else {
    winner_ = new char[strlen(winner) + 1];
    strcpy(winner_, winner);
  }
  teamResults_ = teamResults;
  hasScores_ = hasScores;
  replayBuilder_ = replayBuilder;
  if (errorMessage == 0) {
    errorMessage_ = 0;
  } else {
    errorMessage_ = new char[strlen(errorMessage) + 1];
    strcpy(errorMessage_, errorMessage);
  }
}

MatchResult::~MatchResult() {
  delete stageName_;
  for (int x = 0; x < numTeams_; x++) {
    delete teamNames_[x];
  }
  delete teamNames_;
  if (winner_ != 0) {
    delete winner_;
  }
  if (errorMessage_ != 0) {
    delete errorMessage_;
  }
}

const char* MatchResult::getStageName() {
  return stageName_;
}

char** MatchResult::getTeamNames() {
  return teamNames_;
}

int MatchResult::getNumTeams() {
  return numTeams_;
}

const char* MatchResult::getWinner() {
  return winner_;
}

TeamResult** MatchResult::getTeamResults() {
  return teamResults_;
}

bool MatchResult::hasScores() {
  return hasScores_;
}

ReplayBuilder* MatchResult::getReplayBuilder() {
  return replayBuilder_;
}

bool MatchResult::errored() {
  return (errorMessage_ != 0);
}

const char* MatchResult::getErrorMessage() {
  return errorMessage_;
}
