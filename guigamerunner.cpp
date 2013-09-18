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
#include <sstream>
#include <wx/wx.h>
#include <platformstl/synch/sleep_functions.h>
#include "basedir.h"
#include "bblua.h"
#include "outputconsole.h"
#include "runnerform.h"
#include "bbrunner.h"
#include "guigamerunner.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

GuiGameRunner::GuiGameRunner(PrintHandler *printHandler, char **stageNames,
    int numStages, char **teamNames, int numTeams, Zipper *zipper,
    const char *replayTemplateDir) {
  runnerName_ = 0;
  printHandler_ = printHandler;
  numFormElements_ = 0;
  stageNames_ = stageNames;
  numStages_ = numStages;
  teamNames_ = teamNames;
  numTeams_ = numTeams;
  zipper_ = zipper;
  threadCount_ = 1;
  started_ = false;
  quitting_ = false;
  runnerState_ = 0;
  bbRunner_ = 0;
  if (replayTemplateDir == 0) {
    replayTemplateDir_ = 0;
  } else {
    replayTemplateDir_ = new char[strlen(replayTemplateDir) + 1];
    strcpy(replayTemplateDir_, replayTemplateDir);
  }
}

GuiGameRunner::~GuiGameRunner() {
  if (runnerName_ != 0) {
    delete runnerName_;
  }
  if (bbRunner_ != 0) {
    delete bbRunner_;
  }
  for (int x = 0; x < numFormElements_; x++) {
    delete formElements_[x];
  }
  if (replayTemplateDir_ != 0) {
    delete replayTemplateDir_;
  }
}

void GuiGameRunner::addStageSelect(const char *name) {
  addFormElement(name, TYPE_STAGE_SELECT, numStages_);
}

void GuiGameRunner::addSingleShipSelect(const char *name) {
  addFormElement(name, TYPE_SINGLE_SHIP_SELECT, numTeams_);
}

void GuiGameRunner::addMultiShipSelect(const char *name) {
  addFormElement(name, TYPE_MULTI_SHIP_SELECT, numTeams_);
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
    element->addStringValue(value);
  } else if (type == TYPE_STAGE_SELECT || type == TYPE_SINGLE_SHIP_SELECT) {
    element->clearValues();
    element->addStringValue(value);
  }
}

void GuiGameRunner::setDefault(const char *name, int value) {
  // TODO: Error handling if the key doesn't exist or is the wrong type.
  RunnerFormElement *element = getFormElement(name);
  if (element->getType() == TYPE_INTEGER_TEXT) {
    element->setIntegerValue(value);
  }
}

bool GuiGameRunner::ok(const char *message) {
  RunnerForm *form = new RunnerForm(runnerName_, formElements_,
      numFormElements_, stageNames_, numStages_, teamNames_, numTeams_,
      message);
  form->Show();
  form->Raise();
  while (!form->isDone() && !quitting_) {
    wxYield();
    platformstl::micro_sleep(10000);
  }
  form->Close();
  if (form->isOk()) {
    for (int x = 0; x < numFormElements_; x++) {
      RunnerFormElement *element = formElements_[x];
      element->clearValues();
      wxControl *control = element->getControl();
      if (element->getType() == TYPE_INTEGER_TEXT) {
        wxString integerString = ((wxTextCtrl *) control)->GetValue();
        long value;
        if (!integerString.ToLong(&value)) {
          value = 0;
        }
        element->setIntegerValue((int) value);
      } else {
        wxListBox *listBox = ((wxListBox *) element->getControl());
        wxArrayInt selectedItems;
        listBox->GetSelections(selectedItems);
        wxArrayInt::const_iterator first = selectedItems.begin();
        wxArrayInt::const_iterator last = selectedItems.end();
        while (first != last) {
          int shipIndex = *first++;
          element->addStringValue(listBox->GetString(shipIndex));
        }
      }
    }
  }
  bool ok = form->isOk();
  form->Destroy();
  return ok;
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

void GuiGameRunner::quit() {
  quitting_ = true;
  if (bbRunner_ != 0) {
    bbRunner_->quit();
    // TODO: pretty sure we need a mutex here
    if (runnerState_ != 0) {
      lua_sethook(runnerState_, abortHook, LUA_MASKCOUNT, 1);
    }
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

void GuiGameRunner::setThreadCount(int threadCount) {
  if (!started_) {
    threadCount_ = std::max(1, threadCount);
  }
}

void GuiGameRunner::queueMatch(const char *stageName, char **teamNames,
                               int numTeams) {
  if (!started_) {
    bbRunner_ = new BerryBotsRunner(threadCount_, zipper_, replayTemplateDir_);
    bbRunner_->setListener(new GuiRefresherListener());
    started_ = true;
  }
  bbRunner_->queueMatch(stageName, teamNames, numTeams);
}

bool GuiGameRunner::started() {
  return started_;
}

bool GuiGameRunner::empty() {
  return (bbRunner_ == 0 ? true : bbRunner_->allResultsProcessed());
}

MatchResult* GuiGameRunner::nextResult() {
  if (bbRunner_ != 0) {
    return bbRunner_->nextResult();
  } else {
    return 0;
  }
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

void GuiGameRunner::deleteReplayBuilder(ReplayBuilder *replayBuilder) {
  bbRunner_->deleteReplayBuilder(replayBuilder);
}

void GuiGameRunner::run(const char *runnerName) {
  if (runnerName_ != 0) {
    delete runnerName_;
  }
  runnerName_ = new char[strlen(runnerName) + 1];
  strcpy(runnerName_, runnerName);

  std::string runnersDir = getRunnersDir();
  initRunnerState(&runnerState_, runnersDir.c_str());
  lua_setprinter(runnerState_, printHandler_);

  bool error = false;
  bool opened = false;
  if (luaL_loadfile(runnerState_, runnerName)) {
    std::stringstream msgStream;
    msgStream << "Error loading game runner file: " << runnerName;
    printHandler_->runnerPrint(msgStream.str().c_str());
    error = true;
  } else {
    if (lua_pcall(runnerState_, 0, 0, 0)) {
      opened = true;
      error = true;
    } else {
      std::stringstream msgStream;
      msgStream << "== Loaded: " << runnerName;
      printHandler_->runnerPrint(msgStream.str().c_str());
      printHandler_->runnerPrint("");
    }
  }

  if (error) {
    const char *luaMessage = lua_tostring(runnerState_, -1);
    printHandler_->runnerPrint(luaMessage);
  } else {
    lua_pushcfunction(runnerState_, traceback);
    int errfunc = lua_gettop(runnerState_);
    lua_getglobal(runnerState_, "run");
    pushGameRunner(runnerState_, this);
    pushRunnerForm(runnerState_, this);
    pushRunnerFiles(runnerState_, this);

    int pcallValue = lua_pcall(runnerState_, 3, 0, errfunc);
    lua_remove(runnerState_, errfunc);
    if (pcallValue != 0) {
      const char *luaMessage = lua_tostring(runnerState_, -1);
      printHandler_->runnerPrint(luaMessage);
    } else {
      printHandler_->runnerPrint("");
      std::stringstream msgStream;
      msgStream << "== Finished: " << runnerName;
      printHandler_->runnerPrint(msgStream.str().c_str());
    }
  }

  if (opened) {
    lua_close(runnerState_);
    runnerState_ = 0;
  }

  if (bbRunner_ != 0) {
    bbRunner_->quit();
  }
  runnerState_ = 0;
}

GuiRefresherListener::GuiRefresherListener() {
  
}

void GuiRefresherListener::refresh() {
  wxYield();
}
