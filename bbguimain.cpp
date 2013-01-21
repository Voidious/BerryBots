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

#include <wx/wx.h>

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "bbwx.h"
#include "stage.h"
#include "bbengine.h"
#include "printhandler.h"
#include "guimanager.h"
#include "basedir.h"

using namespace std;

BerryBotsEngine *engine = 0;
Stage *stage = 0;
PrintHandler *printHandler = 0;

class BerryBotsApp: public wxApp {
  GuiManager *guiManager_;
  public:
    virtual bool OnInit();
    virtual void OnQuit(wxCommandEvent &event);
    virtual void OnNewMatch(wxCommandEvent &event);
    virtual void OnPackageShip(wxCommandEvent &event);
    virtual void OnPackageStage(wxCommandEvent &event);
};

wxIMPLEMENT_APP(BerryBotsApp);

bool BerryBotsApp::OnInit() {
  guiManager_ = new GuiManager();
  guiManager_->loadStages(getStageDir().c_str());
  guiManager_->loadBots(getBotsDir().c_str());
  guiManager_->linkListeners();

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
  guiManager_->quit();
  ExitMainLoop();
}

void BerryBotsApp::OnNewMatch(wxCommandEvent &event) {
  guiManager_->hidePackageShipDialog();
  guiManager_->hidePackageStageDialog();
  guiManager_->showNewMatchDialog();
}

void BerryBotsApp::OnPackageShip(wxCommandEvent &event) {
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageStageDialog();
  guiManager_->showPackageShipDialog();
}

void BerryBotsApp::OnPackageStage(wxCommandEvent &event) {
  guiManager_->hideNewMatchDialog();
  guiManager_->hidePackageShipDialog();
  guiManager_->showPackageStageDialog();
}
