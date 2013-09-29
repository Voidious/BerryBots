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

#include <string.h>
#include <sstream>
#include <algorithm>
#include <wx/wx.h>
#include <wx/webview.h>
#include "bbconst.h"
#include "ResourcePath.hpp"
#include "basedir.h"
#include "filemanager.h"
#include "bbwx.h"
#include "menubarmaker.h"
#include "bbengine.h"
#include "stagepreview.h"

StagePreview::StagePreview(
    const char *stagesBaseDir, const char *stageName, int x, int y,
    MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY,
              "Preview", wxPoint(x, y), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;
  fileManager_ = new FileManager();

#ifdef __WINDOWS__
  SetIcon(wxIcon(BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));
  
  // The 8-9 point default font size in Windows is much smaller than Mac/Linux.
  wxFont windowFont = GetFont();
  if (windowFont.GetPointSize() <= 9) {
    SetFont(windowFont.Larger());
  }
#elif __WXGTK__
  SetIcon(wxIcon(BBICON_128, wxBITMAP_TYPE_PNG));
#endif

  mainPanel_ = new wxPanel(this);
  wxWebView *webView = wxWebView::New(mainPanel_, wxID_ANY);

  BerryBotsEngine *engine =
      new BerryBotsEngine(0, fileManager_, resourcePath().c_str());
  std::string previewUrl;
  try {
    previewUrl = savePreviewReplay(engine, stagesBaseDir, stageName);
  } catch (EngineException *e) {
    wxMessageDialog errorMessage(NULL, e->what(), "Preview failure",
                                 wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete engine;
    delete e;
    return;
  }
  webView->EnableContextMenu(false);
  webView->LoadURL(previewUrl);

  Stage *stage = engine->getStage();
  double stageWidth = stage->getWidth() + (STAGE_MARGIN * 2);
  double stageHeight = stage->getHeight() + (STAGE_MARGIN * 2);
  double previewScale = std::min(1.0,
      std::min(((double) MAX_PREVIEW_WIDTH) / stageWidth,
               ((double) MAX_PREVIEW_HEIGHT) / stageHeight));
  webView->SetSizeHints(previewScale * stageWidth, previewScale * stageHeight);

  mainSizer_ = new wxBoxSizer(wxVERTICAL);
  mainSizer_->Add(mainPanel_, 0, wxEXPAND);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(webView, 0, wxEXPAND);
  topSizer->AddSpacer(8);
  char *description = fileManager_->getStageDescription(
      stagesBaseDir, stageName, getCacheDir().c_str());
  wxSizer *descSizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel_);
  wxStaticText *descCtrl = new wxStaticText(mainPanel_, wxID_ANY, description);
  descSizer->Add(descCtrl);
  delete description;
  topSizer->Add(descSizer, 0, wxEXPAND);
  
  wxBoxSizer *borderSizer = new wxBoxSizer(wxVERTICAL);
  borderSizer->Add(topSizer, 0, wxALL | wxEXPAND, 12);
  mainPanel_->SetSizerAndFit(borderSizer);
  SetSizerAndFit(mainSizer_);
}

StagePreview::~StagePreview() {
  delete fileManager_;
}

void StagePreview::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
    Fit();
  }
}

std::string StagePreview::savePreviewReplay(BerryBotsEngine *engine,
    const char *stagesBaseDir, const char *stageName)
    throw (EngineException *) {
  engine->initStage(stagesBaseDir, stageName, getCacheDir().c_str());

  Stage *stage = engine->getStage();
  SetTitle(wxString::Format(wxT("Preview: %s"), stage->getName()));
  ReplayBuilder *previewReplay = engine->getReplayBuilder();
  previewReplay->initShips(1, 1);

  Team *previewTeam = new Team;
  strcpy(previewTeam->name, " ");
  previewReplay->addTeamProperties(previewTeam);

  Ship *previewShip = new Ship;
  ShipProperties *properties = new ShipProperties;
  properties->shipR = properties->shipG = properties->shipB = 255;
  properties->laserR = properties->laserB = 0;
  properties->laserG = 255;
  properties->thrusterR = 255;
  properties->thrusterG = properties->thrusterB = 0;
  strcpy(properties->name, " ");
  previewShip->properties = properties;
  previewReplay->addShipProperties(previewShip);

  previewShip->thrusterAngle = previewShip->thrusterForce = 0;
  Point2D *start = stage->getStart();
  previewShip->x = start->getX();
  previewShip->y = start->getY();
  previewShip->alive = true;
  previewShip->showName = previewShip->energyEnabled = false;
  Ship **previewShips = new Ship*[1];
  previewShips[0] = previewShip;
  previewReplay->addShipStates(previewShips, 1);

  delete previewShips;
  delete properties;
  delete previewShip;
  delete previewTeam;

  std::stringstream filenameStream;
  filenameStream << (rand() % 10000000) << ".html";
  previewReplay->saveReplay(getTmpDir().c_str(),
                            filenameStream.str().c_str());

  std::string previewUrl;
  char *previewPath = fileManager_->getFilePath(getTmpDir().c_str(),
                                                filenameStream.str().c_str());
  previewUrl.append(previewPath);
  delete previewPath;
  delete previewReplay;

  return previewUrl;
}