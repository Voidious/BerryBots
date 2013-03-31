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

#include "outputconsole.h"
#include "gamerunner.h"

#define MAX_FORM_ELEMENTS  20

class RunnerFormElement {
  char *name_;
  int type_;
  char** stringValues_;
  int numStringValues_;
  int maxStringValues_;
  int intValue_;
  char** defaultStringValues_;
  int numDefaultStringValues_;
  int defaultIntValue_;

  public:
    RunnerFormElement(const char *name, int type, int maxStringValues);
    ~RunnerFormElement();
    const char *getName();
    int getType();
    void addStringValue(const char *value);
    char** getStringValues();
    int getNumStringValues();
    void addDefaultStringValue(const char *value);
    char** getDefaultStringValues();
    int getNumDefaultStringValues();
    void setIntegerValue(int value);
    int getIntegerValue();
    void setDefaultIntegerValue(int value);
    int getDefaultIntegerValue();
    void clearDefaults();
};

class GuiGameRunner : public GameRunner {
  RunnerFormElement* formElements_[MAX_FORM_ELEMENTS];
  int numFormElements_;
  int numStages_;
  int numShips_;
  OutputConsole *runnerConsole_;

  public:
    GuiGameRunner(OutputConsole *runnerConsole, int numStages, int numShips);
    ~GuiGameRunner();
    virtual void addStageSelect(const char *name);
    virtual void addSingleShipSelect(const char *name);
    virtual void addMultiShipSelect(const char *name);
    virtual void addIntegerText(const char *name);
    virtual void setDefault(const char *name, const char *value);
    virtual void setDefault(const char *name, int value);
    virtual bool ok();
    virtual int getElementType(const char *name);
    virtual char** getStringValues(const char *name);
    virtual int getNumStringValues(const char *name);
    virtual int getIntegerValue(const char *name);
    virtual void run(const char *runnerName);
  private:
    void addFormElement(const char *name, int type, int maxStringValues);
    RunnerFormElement* getFormElement(const char *name);
};

#endif