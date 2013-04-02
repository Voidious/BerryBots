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

#include <iostream>
#include <string.h>
#include "bbutil.h"
#include "cliprinthandler.h"

CliPrintHandler::CliPrintHandler() {
  numTeams_ = 0;
  nextTeamIndex_ = 0;
}

CliPrintHandler::~CliPrintHandler() {
  delete teams_;
  for (int x = 0; x < nextTeamIndex_; x++) {
    delete teamNames_[x];
  }
  delete teamNames_;
}

void CliPrintHandler::setNumTeams(int numTeams) {
  numTeams_ = numTeams;
  teams_ = new Team*[numTeams];
  teamNames_ = new char*[numTeams];
}

void CliPrintHandler::stagePrint(const char *text) {
  std::cout << "Stage: " << text << std::endl;
}

void CliPrintHandler::shipPrint(lua_State *L, const char *text) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->state == L) {
      std::cout << "Ship: " << teamNames_[x] << ": " << text << std::endl;
      break;
    }
  }
}

void CliPrintHandler::runnerPrint(const char *text) {
  std::cout << "Game Runner: " << text << std::endl;
}

void CliPrintHandler::registerTeam(Team *team, const char *name) {
  if (nextTeamIndex_ < numTeams_) {
    teams_[nextTeamIndex_] = team;
    teamNames_[nextTeamIndex_] = new char[strlen(name) + 1];
    strcpy(teamNames_[nextTeamIndex_], name);
    nextTeamIndex_++;
  }
}

void CliPrintHandler::updateTeams(Team** teams) {
  for (int x = 0; x < nextTeamIndex_; x++) {
    Team *team = teams[x];
    delete teamNames_[x];
    teamNames_[x] = new char[strlen(team->name) + 1];
    strcpy(teamNames_[x], team->name);
  }
}

CliStateListener::CliStateListener(CliPrintHandler* cliPrintHandler) {
  cliPrintHandler_ = cliPrintHandler;
}

void CliStateListener::newTeam(Team *team, const char *filename) {
  cliPrintHandler_->registerTeam(team, filename);
}
