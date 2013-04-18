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

#ifndef BERRYBOTS_RUNNER
#define BERRYBOTS_RUNNER

#define MAX_MATCHES    100000
#define SLEEP_INTERVAL  50000 // 0.05s

#include <pthread.h>
#include "bbutil.h"
#include "zipper.h"

class RefresherListener {
  public:
    virtual void refresh() = 0;
    virtual ~RefresherListener() {};
};

class MatchConfig {
  char *stagesDir_;
  char *shipsDir_;
  char *cacheDir_;
  char *stageName_;
  char **teamNames_;
  int numTeams_;
  char *winnerFilename_;
  bool started_;
  bool finished_;
  bool processedResult_;
  TeamResult **teamResults_;
  char *errorMessage_;

  public:
    MatchConfig(const char *stageName, char **teamNames, int numTeams,
                char *stagesDir, char *shipsDir, char *cacheDir);
    ~MatchConfig();
    const char *getStagesDir();
    const char *getShipsDir();
    const char *getCacheDir();
    const char *getStageName();
    char **getTeamNames();
    int getNumTeams();
    const char *getWinnerFilename();
    void setWinnerFilename(const char *name);
    TeamResult** getTeamResults();
    void setTeamResults(TeamResult **teamResults);
    bool isStarted();
    void started();
    bool isFinished();
    void finished();
    bool hasProcessedResult();
    void processedResult();
    const char *getErrorMessage();
    void setErrorMessage(const char *errorMessage);
};

class MatchResult {
  char *stageName_;
  char **teamNames_;
  int numTeams_;
  char *winner_;
  TeamResult **teamResults_;
  char *errorMessage_;

  public:
    MatchResult(const char *stageName, char **teamNames, int numTeams,
                const char *winner, TeamResult **teamResults,
                const char *errorMessage);
    ~MatchResult();
    const char* getStageName();
    char** getTeamNames();
    int getNumTeams();
    const char* getWinner();
    TeamResult** getTeamResults();
    bool errored();
    const char *getErrorMessage();
};

typedef struct {
  MatchConfig* matches[MAX_MATCHES];
  int numMatches;
  int numThreads;
  volatile int matchesRunning;
  volatile bool done;
  Zipper *zipper;
} SchedulerSettings;

typedef struct {
  SchedulerSettings *schedulerSettings;
  MatchConfig *matchConfig;
} MatchSettings;

class BerryBotsRunner {
  char *stagesDir_;
  char *shipsDir_;
  char *cacheDir_;
  pthread_t schedulerThread_;
  SchedulerSettings *schedulerSettings_;
  RefresherListener *listener_;

  public:
    BerryBotsRunner(int threadCount, Zipper *zipper);
    ~BerryBotsRunner();
    void queueMatch(const char *stageName, char **teamNames, int numTeams);
    MatchResult* nextResult();
    bool allResultsProcessed();
    void quit();
    void setListener(RefresherListener *listener);
    static void* scheduler(void *vargs);
    static void* runMatch(void *vargs);
};

#endif
