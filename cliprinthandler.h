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

#ifndef CLI_PRINT_HANDLER_H
#define CLI_PRINT_HANDLER_H

extern "C" {
  #include "lua.h"
}

#include "bbutil.h"
#include "bbengine.h"
#include "printhandler.h"

class CliPrintHandler : public PrintHandler {
  Team **teams_;
  char **teamNames_;
  int numTeams_;
  int nextTeamIndex_;

  public:
    CliPrintHandler();
    ~CliPrintHandler();
    void setNumTeams(int numTeams);
    virtual void stagePrint(const char *text);
    virtual void shipPrint(lua_State *L, const char *text);
    virtual void runnerPrint(const char *text);
    virtual void registerTeam(Team *team, const char *filename);
    void updateTeams(Team** teams);
};

#endif
