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

#ifndef PRINT_HANDLER_H
#define PRINT_HANDLER_H

extern "C" {
  #include "lua.h"
}

// Custom implementation of the print() Lua function to accomodate command line
// and GUI implementations of BerryBots stage/ship output.
//
// Note that the PrintHandler is the only global in the BerryBots code base. So
// while the game is fairly thread-safe - if using one engine per thread, and
// still lacking mutexes on filesystem access - the PrintHandler is shared
// across all threads and engines.

class PrintHandler {
  public:
    virtual void stagePrint(const char *text) = 0;
    virtual void shipPrint(lua_State *L, const char *text) = 0;
    virtual ~PrintHandler() {};
};

#endif
