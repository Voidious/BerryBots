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

StagePreview::StagePreview(const char *stagesBaseDir,
                           MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, "Preview", wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;
  fileManager_ = new FileManager();
  listener_ = 0;
  stagesBaseDir_ = new char[strlen(stagesBaseDir) + 1];
  strcpy(stagesBaseDir_, stagesBaseDir);
  stageName_ = 0;

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
  webView_ = wxWebView::New(mainPanel_, wxID_ANY);
  webView_->EnableContextMenu(false);
  webView_->EnableHistory(false);

  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(mainPanel_, 0, wxEXPAND);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(webView_, 0, wxEXPAND);
  topSizer->AddSpacer(8);
  descSizer_ = new wxStaticBoxSizer(wxVERTICAL, mainPanel_);
  topSizer->Add(descSizer_, 0, wxEXPAND);
  
  wxBoxSizer *borderSizer = new wxBoxSizer(wxVERTICAL);
  borderSizer->Add(topSizer, 0, wxALL | wxEXPAND, 12);
  mainPanel_->SetSizerAndFit(borderSizer);
  SetSizerAndFit(mainSizer);

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(StagePreview::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(StagePreview::onClose));
  Connect(webView_->GetId(), wxEVT_WEBVIEW_LOADED,
          wxWebViewEventHandler(StagePreview::onLoaded));
          
  eventFilter_ = new PreviewEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

StagePreview::~StagePreview() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  if (listener_ != 0) {
    delete listener_;
  }
  delete eventFilter_;
  delete fileManager_;
  delete stagesBaseDir_;
  if (stageName_ != 0) {
    delete stageName_;
  }
}

void StagePreview::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
    Fit();
  }
}

void StagePreview::onClose(wxCommandEvent &event) {
  if (listener_ != 0) {
    listener_->onClose();
  }
  Hide();
}

void StagePreview::onLoaded(wxWebViewEvent &event) {
  if (listener_ != 0 && stageName_ != 0) {
    listener_->onLoaded(stageName_);
  }
  Show();
  Raise();
}

void StagePreview::onUp() {
  if (listener_ != 0) {
    listener_->onUp();
  }
}

void StagePreview::onDown() {
  if (listener_ != 0) {
    listener_->onDown();
  }
}

void StagePreview::setListener(StagePreviewListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

void StagePreview::showPreview(const char *stageName, int x, int y) {
  Hide();
  if (stageName_ != 0) {
    delete stageName_;
  }
  stageName_ = new char[strlen(stageName) + 1];
  strcpy(stageName_, stageName);

  SetPosition(wxPoint(x, y));
  BerryBotsEngine *engine =
      new BerryBotsEngine(0, fileManager_, resourcePath().c_str());
  std::string previewUrl;
  try {
    previewUrl = savePreviewReplay(engine, stagesBaseDir_, stageName);
  } catch (EngineException *e) {
    wxMessageDialog errorMessage(NULL, e->what(), "Preview failure",
                                 wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete engine;
    delete e;
    return;
  }
  webView_->LoadURL(previewUrl);

  Stage *stage = engine->getStage();
  double stageWidth = stage->getWidth() + (STAGE_MARGIN * 2);
  double stageHeight = stage->getHeight() + (STAGE_MARGIN * 2);
  double previewScale = std::min(1.0,
      std::min(((double) MAX_PREVIEW_WIDTH) / stageWidth,
               ((double) MAX_PREVIEW_HEIGHT) / stageHeight));
  webView_->SetSizeHints(previewScale * stageWidth, previewScale * stageHeight);
  delete engine;

  char *description = fileManager_->getStageDescription(
      stagesBaseDir_, stageName, getCacheDir().c_str());
  wxStaticText *descCtrl = new wxStaticText(mainPanel_, wxID_ANY, description);
  descSizer_->Clear(true);
  descSizer_->Add(descCtrl);
  delete description;

  mainPanel_->GetSizer()->SetSizeHints(mainPanel_);
  Fit();
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
  delete start;

  std::stringstream filenameStream;
  filenameStream << (rand() % 10000000) << ".html";
  previewReplay->setExtraJavascript("BerryBots.interactive = false;");
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

PreviewEventFilter::PreviewEventFilter(StagePreview *stagePreview) {
  stagePreview_ = stagePreview;
}

int PreviewEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && stagePreview_->IsActive()) {
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE || keyCode == WXK_SPACE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      stagePreview_->Close();
      return Event_Processed;
    } else if (keyCode == WXK_UP) {
      stagePreview_->onUp();
    } else if (keyCode == WXK_DOWN) {
      stagePreview_->onDown();
    }
  }

  return Event_Skip;
}
