/*
  Copyright (C) 2012-2013 - Voidious

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

#include "bbutil.h"
#include "menubarmaker.h"
#include "outputconsole.h"
#include "guiprinthandler.h"

GuiPrintHandler::GuiPrintHandler(OutputConsole *stageConsole,
    OutputConsole *runnerConsole, MenuBarMaker *menuBarMaker) {
  stageConsole_ = stageConsole;
  runnerConsole_ = runnerConsole;
  menuBarMaker_ = menuBarMaker;
  nextTeamIndex_ = numTeams_ = 0;
  restartMode_ = false;
}

GuiPrintHandler::~GuiPrintHandler() {
  for (int x = 0; x < numTeams_; x++) {
    teamConsoles_[x]->Hide();
    delete teamConsoles_[x];
  }
}

void GuiPrintHandler::stagePrint(const char *text) {
  if (stageConsole_ != 0) {
    stageConsole_->println(text);
  }
}

void GuiPrintHandler::shipPrint(lua_State *L, const char *text) {
  for (int x = 0; x < numTeams_; x++) {
    if (teams_[x]->state == L) {
      teamConsoles_[x]->println(text);
      break;
    }
  }
}

void GuiPrintHandler::runnerPrint(const char *text) {
  if (runnerConsole_ != 0) {
    runnerConsole_->println(text);
  }
}

void GuiPrintHandler::registerTeam(Team *team, const char *filename) {
  OutputConsole *teamConsole = 0;
  if (restartMode_) {
    if (nextTeamIndex_ >= numTeams_) {
      // Restarting should never have more teams than last time.
      exit(EXIT_FAILURE);
    }
    teams_[nextTeamIndex_] = team;
    teamConsole = teamConsoles_[nextTeamIndex_];
    team->gfxEnabled = teamConsole->isChecked();
    teamConsole->setListener(new TeamConsoleListener(team));
    teamConsole->clear();
    nextTeamIndex_++;
  } else {
    if (numTeams_ < MAX_TEAM_CONSOLES) {
      teams_[numTeams_] = team;
      teamConsole = new OutputConsole(filename, true, menuBarMaker_);
      teamConsole->setListener(new TeamConsoleListener(team));
      teamConsole->Hide();
      teamConsoles_[numTeams_] = teamConsole;
      nextTeamIndex_ = numTeams_++;
    }
  }
  if (teamConsole != 0) {
    teamConsole->print("== Ship control program loaded: ");
    teamConsole->println(filename);
  }
}

void GuiPrintHandler::restartMode() {
  nextTeamIndex_ = 0;
  restartMode_ = true;
}

OutputConsole** GuiPrintHandler::getTeamConsoles() {
  return teamConsoles_;
}

TeamConsoleListener::TeamConsoleListener(Team *team) {
  team_ = team;
}

void TeamConsoleListener::onActive() {
  team_->errored = false;
}

void TeamConsoleListener::onCheck(bool checked) {
  team_->gfxEnabled = checked;
}
