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
#include <math.h>
#include <SFML/Graphics.hpp>
#include <wx/wx.h>

#include "bbwx.h"
#include "stage.h"
#include "bbengine.h"
#include "printhandler.h"
#include "guimanager.h"
#include "bbutil.h"

using namespace std;

BerryBotsEngine *engine = 0;
Stage *stage = 0;
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

#ifdef __WXOSX__
  // On OS X, it complains if we initialize our first SFML window after the
  // wxWidgets windows have set their menu bars or after the base dir selector,
  // so initialize one here first.
  sf::RenderWindow *window = new sf::RenderWindow(
      sf::VideoMode(800, 600), "BerryBots", sf::Style::Default,
      sf::ContextSettings(0, 0, 0, 2, 0));
  delete window;
#endif

  char *stageDir = guiManager_->getStageDirCopy();
  char *botsDir = guiManager_->getBotsDirCopy();
  guiListener_ = new AppGuiListener(this);
  guiManager_ = new GuiManager(guiListener_, stageDir, botsDir);
  delete stageDir;
  delete botsDir;

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnQuit));
  Connect(NEW_MATCH_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnNewMatch));
  Connect(PACKAGE_SHIP_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageShip));
  Connect(PACKAGE_STAGE_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageStage));

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
}

void BerryBotsApp::OnPackageShip(wxCommandEvent &event) {
  guiManager_->showPackageShipDialog();
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageStageDialog();
}

void BerryBotsApp::OnPackageStage(wxCommandEvent &event) {
  guiManager_->showPackageStageDialog();
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageShipDialog();
}

AppGuiListener::AppGuiListener(BerryBotsApp *app) {
  app_ = app;
}

AppGuiListener::~AppGuiListener() {
  
}

void AppGuiListener::onAllWindowsClosed() {
  app_->quit();
}
