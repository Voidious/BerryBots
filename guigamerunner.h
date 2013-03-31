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

class GuiGameRunner : public GameRunner {
  OutputConsole *runnerConsole_;

  public:
    GuiGameRunner(OutputConsole *runnerConsole);
    ~GuiGameRunner();
    virtual void addStageSelect(const char *name);
    virtual void addSingleShipSelect(const char *name);
    virtual void addMultiShipSelect(const char *name);
    virtual void addIntegerText(const char *name);
    virtual void setDefault(const char *name, const char *value);
    virtual void setDefault(const char *name, int value);
    virtual bool ok();
    virtual int getType(const char *name);
    virtual const char* getString(const char *name);
    virtual int getInteger(const char *name);
    virtual void run(const char *runnerName);
};

#endif
