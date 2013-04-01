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
#include "basedir.h"
#include "bblua.h"
#include "outputconsole.h"
#include "runnerform.h"
#include "guigamerunner.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

GuiGameRunner::GuiGameRunner(OutputConsole *runnerConsole, int numStages,
                             int numShips) {
  runnerConsole_ = runnerConsole;
  numFormElements_ = 0;
  numStages_ = numStages;
  numShips_ = numShips;
}

GuiGameRunner::~GuiGameRunner() {

}

void GuiGameRunner::addStageSelect(const char *name) {
  addFormElement(name, TYPE_STAGE_SELECT, numStages_);
}

void GuiGameRunner::addSingleShipSelect(const char *name) {
  addFormElement(name, TYPE_SINGLE_SHIP_SELECT, numShips_);
}

void GuiGameRunner::addMultiShipSelect(const char *name) {
  addFormElement(name, TYPE_MULTI_SHIP_SELECT, numShips_);
}

void GuiGameRunner::addIntegerText(const char *name) {
  addFormElement(name, TYPE_INTEGER_TEXT, 0);
}

// TODO: Document limit and gracefully handle adding too many form elements.
void GuiGameRunner::addFormElement(const char *name, int type,
                                   int maxStringValues) {
  if (numFormElements_ < MAX_FORM_ELEMENTS) {
    formElements_[numFormElements_++] =
        new RunnerFormElement(name, type, maxStringValues);
  }
}

void GuiGameRunner::setDefault(const char *name, const char *value) {
  // TODO: Error handling if the key doesn't exist or is the wrong type.
  RunnerFormElement *element = getFormElement(name);
  int type = element->getType();
  if (type == TYPE_MULTI_SHIP_SELECT) {
    element->addDefaultStringValue(value);
  } else if (type == TYPE_STAGE_SELECT || type == TYPE_SINGLE_SHIP_SELECT) {
    element->clearDefaults();
    element->addDefaultStringValue(value);
  }
}

void GuiGameRunner::setDefault(const char *name, int value) {
  // TODO: Error handling if the key doesn't exist or is the wrong type.
  RunnerFormElement *element = getFormElement(name);
  if (element->getType() == TYPE_INTEGER_TEXT) {
    element->setDefaultIntegerValue(value);
  }
}

bool GuiGameRunner::ok() {
  return false;
}

int GuiGameRunner::getElementType(const char *name) {
  RunnerFormElement *element = getFormElement(name);
  if (element == 0) {
    return RUNNER_UNDEFINED;
  } else {
    return element->getType();
  }
}

char** GuiGameRunner::getStringValues(const char *name) {
  RunnerFormElement *element = getFormElement(name);
  if (element == 0) {
    return 0;
  } else {
    return element->getStringValues();
  }
}

int GuiGameRunner::getNumStringValues(const char *name) {
  RunnerFormElement *element = getFormElement(name);
  if (element == 0) {
    return 0;
  } else {
    return element->getNumStringValues();
  }
}

int GuiGameRunner::getIntegerValue(const char *name) {
  RunnerFormElement *element = getFormElement(name);
  if (element == 0) {
    return 0;
  } else {
    return element->getIntegerValue();
  }
}

RunnerFormElement* GuiGameRunner::getFormElement(const char *name) {
  // TODO: Error handling if the key doesn't exist or is the wrong type.
  for (int x = 0; x < numFormElements_; x++) {
    RunnerFormElement *element = formElements_[x];
    if (strcmp(element->getName(), name) == 0) {
      return element;
    }
  }
  return 0;
}

// Taken from luajit.c
static int traceback(lua_State *L) {
  if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
    if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring")
        || !lua_isstring(L, -1)) {
      return 1;  /* Return non-string error object. */
    }
    lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
  }
  luaL_traceback(L, L, lua_tostring(L, 1), 1);
  return 1;
}

void GuiGameRunner::run(const char *runnerName) {
  runnerConsole_->clear();
  runnerConsole_->Show();
  runnerConsole_->Raise();

  lua_State *runnerState;
  std::string runnersDir = getRunnersDir();
  initRunnerState(&runnerState, runnersDir.c_str());

  bool error = false;
  bool opened = false;
  if (luaL_loadfile(runnerState, runnerName)) {
    runnerConsole_->print("Error loading game runner file: ");
    runnerConsole_->println(runnerName);
    error = true;
  } else {
    if (lua_pcall(runnerState, 0, 0, 0)) {
      opened = true;
      error = true;
    } else {
      runnerConsole_->print("Loaded: ");
      runnerConsole_->println(runnerName);
      runnerConsole_->println();
    }
  }

  if (error) {
    const char *luaMessage = lua_tostring(runnerState, -1);
    runnerConsole_->println(luaMessage);
  } else {
    lua_pushcfunction(runnerState, traceback);
    int errfunc = lua_gettop(runnerState);
    lua_getglobal(runnerState, "run");
    pushRunnerForm(runnerState, this);
    pushGameRunner(runnerState, this);
    pushRunnerFiles(runnerState, this);

    int pcallValue = lua_pcall(runnerState, 3, 0, errfunc);
    lua_remove(runnerState, errfunc);
    if (pcallValue != 0) {
      const char *luaMessage = lua_tostring(runnerState, -1);
      runnerConsole_->println(luaMessage);
    } else {
      runnerConsole_->println();
      runnerConsole_->print("Finished: ");
      runnerConsole_->println(runnerName);
    }
  }

  if (opened) {
    lua_close(runnerState);
  }
}
