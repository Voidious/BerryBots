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

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sstream>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <wx/wx.h>

#include "bbwx.h"
#include "stage.h"
#include "bbengine.h"
#include "printhandler.h"
#include "guimanager.h"
#include "bbutil.h"
#include "basedir.h"

using namespace std;

PrintHandler *printHandler = 0;

class BerryBotsApp: public wxApp {
  GuiManager *guiManager_;
  GuiListener *guiListener_;

  public:
    virtual bool OnInit();
    virtual void OnQuit(wxCommandEvent &event);
    void quit();
    virtual void OnNewMatch(wxCommandEvent &event);
    virtual void OnPackageShip(wxCommandEvent &event);
    virtual void OnPackageStage(wxCommandEvent &event);
    virtual void OnErrorConsole(wxCommandEvent &event);
    virtual void MacReopenApp();
};

class AppGuiListener : public GuiListener {
  BerryBotsApp *app_;
  
  public:
    AppGuiListener(BerryBotsApp *app);
    ~AppGuiListener();
    void onAllWindowsClosed();
};

wxIMPLEMENT_APP(BerryBotsApp);

bool BerryBotsApp::OnInit() {

#ifdef __WXGTK__
  // Needed to load dock icons for wxWidgets dialogs.
  wxImage::AddHandler(new wxPNGHandler);
#endif

#ifdef __WXOSX__
  // On OS X, it complains if we initialize our first SFML window after the
  // wxWidgets windows have set their menu bars or after the base dir selector,
  // so initialize one here first.
  sf::RenderWindow *window = new sf::RenderWindow(
      sf::VideoMode(800, 600), "BerryBots", sf::Style::Default,
      sf::ContextSettings(0, 0, 0, 2, 0));
  delete window;

  // On OS X, if base directory has not been selected yet on OS, throw up an
  // informational dialog with wxWidgets before the file open dialog.
  if (!isConfigured()) {
    std::stringstream configInfo;
    configInfo << "TLDR: Before you can use BerryBots, you need to select a "
               << "base directory. This is where BerryBots will read and write "
               << "files like ships and stages."
               << std::endl << std::endl
               << "Home > Documents > BerryBots is a reasonable choice."
               << std::endl << std::endl
               << "---"
               << std::endl << std::endl
               << "After selecting a directory, subdirectories will be created "
               << "for ships (bots/) and stages (stages/). The sample ships "
               << "and stages will be copied over. Place your own ships and "
               << "stages there, too."
               << std::endl << std::endl
               << "The BerryBots Lua API documentation will be copied into the "
               << "apidoc/ subdirectory."
               << std::endl << std::endl
               << "As needed, BerryBots will also create a cache (cache/) "
               << "subdirectory for extracting packaged ships and stages, as "
               << "well as a temp (.tmp/) subdirectory for working files used "
               << "to package ships and stages."
               << std::endl << std::endl
               << "Have fun!";
    
    wxMessageDialog selectBaseDirMessage(NULL, configInfo.str(),
                                        "BerryBots Setup", wxOK);
    if (selectBaseDirMessage.ShowModal() != wxID_OK) {
      return false;
    }
  }
#endif

  guiListener_ = new AppGuiListener(this);
  guiManager_ = new GuiManager(guiListener_);

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnQuit));
  Connect(NEW_MATCH_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnNewMatch));
  Connect(PACKAGE_SHIP_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageShip));
  Connect(PACKAGE_STAGE_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageStage));
  Connect(ERROR_CONSOLE_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnErrorConsole));
#ifndef __WXOSX__
  Connect(FILE_QUIT_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnQuit));
#endif

  return true;
}

void BerryBotsApp::OnQuit(wxCommandEvent &event) {
  quit();
}

void BerryBotsApp::quit() {
  guiManager_->quit();
  ExitMainLoop();
  delete guiListener_;
  // TODO: delete guiManager_ without clobbering anyone else
}

void BerryBotsApp::OnNewMatch(wxCommandEvent &event) {
  guiManager_->showNewMatchDialog();
  guiManager_->hidePackageShipDialog();
  guiManager_->hidePackageStageDialog();
  guiManager_->newMatchInitialFocus();
}

void BerryBotsApp::OnPackageShip(wxCommandEvent &event) {
  guiManager_->showPackageShipDialog();
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageStageDialog();
  guiManager_->packageShipInitialFocus();
}

void BerryBotsApp::OnPackageStage(wxCommandEvent &event) {
  guiManager_->showPackageStageDialog();
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageShipDialog();
  guiManager_->packageStageInitialFocus();
}

void BerryBotsApp::OnErrorConsole(wxCommandEvent &event) {
  guiManager_->showErrorConsole();
}

void BerryBotsApp::MacReopenApp() {
  // No-op - When SFML window is only one open and user switches to app via icon
  // in Mac OS X dock, this stops wxWidgets from opening one of its windows
  // unnecessarily.
}

AppGuiListener::AppGuiListener(BerryBotsApp *app) {
  app_ = app;
}

AppGuiListener::~AppGuiListener() {
  
}

void AppGuiListener::onAllWindowsClosed() {
  app_->quit();
}
