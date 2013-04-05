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

#define MAX_MATCHES  100000

#include <pthread.h>

class MatchConfig {
  char *stageName_;
  char **shipNames_;
  int numShips_;
  char *winnerName_; // TODO: replace with collection of results and filename
  bool processedResult_;

  public:
    MatchConfig(const char *stageName, char **shipNames, int numShips);
    ~MatchConfig();
    const char *getStageName();
    char **getShipNames();
    int getNumShips();
    const char *getWinnerName();
    void setWinnerName(const char *name);
    bool getProcessedResult();
    void setProcessedResult();
};

class BerryBotsRunner {
  int threadCount_;
  pthread_t *threads_;
  MatchConfig* matches_[MAX_MATCHES];
  int numMatches_;

  public:
    BerryBotsRunner(int threadCount);
    ~BerryBotsRunner();
    void queueMatch(const char *stageName, char **shipNames, int numShips);
};

#endif
