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

#include <wx/wx.h>
#include "bbwx.h"
#include "menubarmaker.h"

MenuBarMaker::MenuBarMaker() {
  
}

wxMenuBar* MenuBarMaker::getNewMenuBar() {
  wxMenu *fileMenu = new wxMenu();
  fileMenu->Insert(0, NEW_MATCH_MENU_ID, "&New Match...\tCtrl+N", 0);
  fileMenu->Insert(1, PACKAGE_SHIP_MENU_ID, "&Package Ship...\tCtrl+P", 0);
  fileMenu->Insert(2, PACKAGE_STAGE_MENU_ID, "Package S&tage...\tCtrl+T", 0);
#ifdef __WXOSX__
  fileMenu->Insert(3, GAME_RUNNER_MENU_ID, "&Game Runner...\tShift+Ctrl+G", 0);
#else
  fileMenu->Insert(3, GAME_RUNNER_MENU_ID, "&Game Runner...\tShift+Alt+G", 0);
#endif
  fileMenu->Insert(4, ERROR_CONSOLE_MENU_ID, "&Error Console\tCtrl+E", 0);
  fileMenu->InsertSeparator(5);
#ifdef __WXOSX__
  fileMenu->Insert(6, CHANGE_BASE_DIR_MENU_ID,
                   "Change &Base Directory\tCtrl+B");
#else
  fileMenu->Insert(6, FILE_QUIT_MENU_ID, "&Quit");
#endif
  
  wxMenu *browseMenu = new wxMenu();
#ifdef __WXOSX__
  browseMenu->Insert(0, BROWSE_SHIPS_MENU_ID,
                     "&Ships Directory\tShift+Ctrl+S", 0);
  browseMenu->Insert(1, BROWSE_STAGES_MENU_ID,
                     "S&tages Directory\tShift+Ctrl+T", 0);
  browseMenu->Insert(2, BROWSE_RUNNERS_MENU_ID,
                     "&Runners Directory\tShift+Ctrl+R", 0);
  browseMenu->Insert(3, BROWSE_API_DOCS_MENU_ID,
                     "&API Docs\tShift+Ctrl+A", 0);
#else
  browseMenu->Insert(0, BROWSE_SHIPS_MENU_ID,
                     "&Ships Directory\tShift+Alt+S", 0);
  browseMenu->Insert(1, BROWSE_STAGES_MENU_ID,
                     "S&tages Directory\tShift+Alt+T", 0);
  browseMenu->Insert(2, BROWSE_RUNNERS_MENU_ID,
                     "&Runners Directory\tShift+Alt+R", 0);
  browseMenu->Insert(3, BROWSE_API_DOCS_MENU_ID,
                     "&API Docs\tShift+Alt+A", 0);
#endif
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Insert(0, fileMenu, "&File");
  menuBar->Insert(1, browseMenu, "&Browse");

  return menuBar;
}
