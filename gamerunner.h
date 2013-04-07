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

#ifndef GAME_RUNNER_H
#define GAME_RUNNER_H

#include "bbrunner.h"

#define RUNNER_UNDEFINED        -1
#define TYPE_STAGE_SELECT        1
#define TYPE_SINGLE_SHIP_SELECT  2
#define TYPE_MULTI_SHIP_SELECT   3
#define TYPE_INTEGER_TEXT        4
#define TYPE_OK_CANCEL           5

class GameRunner {
  public:
    virtual void addStageSelect(const char *name) = 0;
    virtual void addSingleShipSelect(const char *name) = 0;
    virtual void addMultiShipSelect(const char *name) = 0;
    virtual void addIntegerText(const char *name) = 0;
    virtual void setDefault(const char *name, const char *value) = 0;
    virtual void setDefault(const char *name, int value) = 0;
    virtual bool ok() = 0;
    virtual int getElementType(const char *name) = 0;
    virtual char** getStringValues(const char *name) = 0;
    virtual int getNumStringValues(const char *name) = 0;
    virtual int getIntegerValue(const char *name) = 0;
    virtual void setThreadCount(int threadCount) = 0;
    virtual void queueMatch(const char *stageName, char **teamNames,
                            int numTeams) = 0;
    virtual bool started() = 0;
    virtual bool empty() = 0;
    virtual MatchResult* nextResult() = 0;
    virtual void run(const char *runnerName) = 0;
    virtual ~GameRunner() {};
};

#endif
