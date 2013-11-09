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

#ifndef GUI_GAME_RUNNER_H
#define GUI_GAME_RUNNER_H

#include "runnerform.h"
#include "bbrunner.h"
#include "zipper.h"
#include "printhandler.h"
#include "replaybuilder.h"
#include "gamerunner.h"

#define MAX_FORM_ELEMENTS  20

class GuiGameRunner : public GameRunner {
  char *runnerName_;
  RunnerFormElement* formElements_[MAX_FORM_ELEMENTS];
  int numFormElements_;
  char **stageNames_;
  int numStages_;
  char **teamNames_;
  int numTeams_;
  PrintHandler *printHandler_;
  int threadCount_;
  bool started_;
  bool quitting_;
  lua_State *runnerState_;
  BerryBotsRunner *bbRunner_;
  Zipper *zipper_;
  char *replayTemplateDir_;
  RunnerFormListener *runnerFormListener_;

  public:
    GuiGameRunner(PrintHandler *printHandler, char **stageNames, int numStages,
        char **teamNames, int numTeams, Zipper *zipper,
        const char *replayTemplateDir, RunnerFormListener *runnerFormListener);
    ~GuiGameRunner();
    virtual void addStageSelect(const char *name);
    virtual void addSingleShipSelect(const char *name);
    virtual void addMultiShipSelect(const char *name);
    virtual void addIntegerText(const char *name);
    virtual void addCheckbox(const char *name);
    virtual void setDefault(const char *name, const char *value);
    virtual void setDefault(const char *name, int value);
    virtual void setDefault(const char *name, bool value);
    virtual bool ok(const char *message);
    virtual int getElementType(const char *name);
    virtual char** getStringValues(const char *name);
    virtual int getNumStringValues(const char *name);
    virtual int getIntegerValue(const char *name);
    virtual bool getBooleanValue(const char *name);
    virtual void setThreadCount(int threadCount);
    virtual void queueMatch(const char *stageName, char **teamNames,
                            int numTeams);
    virtual bool started();
    virtual bool empty();
    virtual MatchResult* nextResult();
    virtual void deleteReplayBuilder(ReplayBuilder *replayBuilder);
    virtual void run(const char *runnerName);
    void quit();
  private:
    void addFormElement(const char *name, int type, int maxStringValues);
    RunnerFormElement* getFormElement(const char *name);
};

class GuiRefresherListener : public RefresherListener {
  public:
    GuiRefresherListener();
    virtual void refresh();
};

#endif
