/*
  Copyright (C) 2012 - Voidious

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
#include "guiprinthandler.h"

GuiPrintHandler::GuiPrintHandler(OutputConsole *stageConsole,
    OutputConsole **teamConsoles, Team **teams, int numTeams) {
  stageConsole_ = stageConsole;
  teamConsoles_ = teamConsoles;
  teams_ = teams;
  numTeams_ = numTeams;
}

void GuiPrintHandler::stagePrint(const char *text) {
  stageConsole_->println(text);
}

void GuiPrintHandler::shipPrint(lua_State *L, const char *text) {
  for (int x = 0; x < numTeams_; x++) {
    Team *team = teams_[x];
    if (team->state == L) {
      teamConsoles_[x]->println(text);
      break;
    }
  }
}
