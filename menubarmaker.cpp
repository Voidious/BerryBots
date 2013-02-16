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
  fileMenu->Insert(3, ERROR_CONSOLE_MENU_ID, "&Error Console\tCtrl+E", 0);
#ifndef __WXOSX__
  fileMenu->InsertSeparator(4);
  fileMenu->Insert(5, FILE_QUIT_MENU_ID, "&Quit");
#endif
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Insert(0, fileMenu, "&File");
  return menuBar;
}
