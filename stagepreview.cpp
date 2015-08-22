/*
  Copyright (C) 2013-2015 - Voidious

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
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <wx/wx.h>
#include "bbconst.h"
#include "bbwx.h"
#include "ResourcePath.hpp"
#include "basedir.h"
#include "filemanager.h"
#include "menubarmaker.h"
#include "bbengine.h"
#include "gfxmanager.h"
#include "stagepreview.h"

StagePreview::StagePreview(MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, "Preview", wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;
  fileManager_ = new FileManager();
  listener_ = 0;
  previewGfxManager_ = new GfxManager(resourcePath(), false);
  stageName_ = 0;

#ifdef __WINDOWS__
  SetIcon(wxIcon(resourcePath() + BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));

  // The 8-9 point default font size in Windows is much smaller than Mac/Linux.
  wxFont windowFont = GetFont();
  if (windowFont.GetPointSize() <= 9) {
    SetFont(windowFont.Larger());
  }
#elif __WXGTK__
  SetIcon(wxIcon(resourcePath() + BBICON_128, wxBITMAP_TYPE_PNG));
#endif

  mainPanel_ = new wxPanel(this);
  infoSizer_ = new wxStaticBoxSizer(wxVERTICAL, mainPanel_);
  descSizer_ = new wxStaticBoxSizer(wxVERTICAL, mainPanel_);

  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(mainPanel_, 0, wxEXPAND);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxImage iconImg;
  iconImg.LoadFile(std::string(resourcePath() + BBICON_128).c_str());
  visualPreview_ = new wxStaticBitmap(mainPanel_, wxID_ANY, wxBitmap(iconImg));
  topSizer->Add(visualPreview_, 0, wxALIGN_CENTER);
  topSizer->AddSpacer(8);

  wxBoxSizer *rowSizer = new wxBoxSizer(wxHORIZONTAL);
  rowSizer->Add(infoSizer_, 0, wxEXPAND);
  rowSizer->AddSpacer(4);
  rowSizer->Add(descSizer_, 1, wxEXPAND);
  topSizer->Add(rowSizer, 0, wxEXPAND);

  wxBoxSizer *borderSizer = new wxBoxSizer(wxVERTICAL);
  borderSizer->Add(topSizer, 0, wxALL | wxEXPAND, 12);
  mainPanel_->SetSizerAndFit(borderSizer);
  SetSizerAndFit(mainSizer);

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(StagePreview::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(StagePreview::onClose));

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
  if (stageName_ != 0) {
    delete stageName_;
  }
  delete previewGfxManager_;
}

void StagePreview::onActivate(wxActivateEvent &event) {
#ifndef __WINDOWS__
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
  }
#endif
  Fit();
}

void StagePreview::onClose(wxCommandEvent &event) {
  if (listener_ != 0) {
    listener_->onClose();
  }
  Hide();
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

void StagePreview::showPreview(sf::RenderWindow *window, const char *stageName,
                               int x, int y) {
  if (stageName_ != 0) {
    delete stageName_;
  }
  stageName_ = new char[strlen(stageName) + 1];
  strcpy(stageName_, stageName);

  SetPosition(wxPoint(x, y));
  infoSizer_->Clear(true);
  descSizer_->Clear(true);
  
  BerryBotsEngine *engine = new BerryBotsEngine(0, fileManager_, 0);
  Stage *stage = engine->getStage();
  try {
    engine->initStage(getStagesDir().c_str(), stageName, getCacheDir().c_str());
  } catch (EngineException *e) {
    wxMessageDialog errorMessage(NULL, e->what(), "Preview failure",
                                 wxOK | wxICON_EXCLAMATION);
    errorMessage.ShowModal();
    delete engine;
    delete e;
    return;
  }
  SetTitle(wxString::Format(wxT("%s"), stage->getName()));
  unsigned int targetWidth;
  unsigned int targetHeight;
  char *previewFilename =
      savePreviewImage(window, engine, targetWidth, targetHeight);
  wxImage previewImage;
  previewImage.LoadFile(previewFilename);
  visualPreview_->SetMinSize(wxSize(targetWidth, targetHeight));
  visualPreview_->SetMaxSize(wxSize(targetWidth, targetHeight));
  delete previewFilename;

#ifdef __WXOSX__
  int padding = 4;
#else
  int padding = 8;
#endif
  wxSizer *infoGrid = new wxFlexGridSizer(2, 0, padding);
  addInfo(infoGrid, "Name:", stage->getName());
  addInfo(infoGrid, "Size:",
      wxString::Format(wxT("%i x %i"), stage->getWidth(), stage->getHeight()));
  if (engine->getTeamSize() > 1) {
    addInfo(infoGrid, "Team size:", engine->getTeamSize());
  }
  addInfo(infoGrid, "Walls:", (stage->getWallCount() - 4));
  addInfo(infoGrid, "Zones:", stage->getZoneCount());
  addInfo(infoGrid, "Starts:", stage->getStartCount());
  int numStageShips = stage->getStageShipCount();
  if (numStageShips > 0) {
    char **stageShips = stage->getStageShips();
    for (int x = 0; x < numStageShips; x++) {
      const char *shipName = stageShips[x];
      if (shipName != 0) {
        int count = 1;
        for (int y = x + 1; y < numStageShips; y++) {
          const char *shipName2 = stageShips[y];
          if (shipName2 != 0 && strcmp(shipName, shipName2) == 0) {
            count++;
            stageShips[y] = 0;
          }
        }
        wxString wxShipName = (count == 1) ? wxString(stageShips[x])
            : wxString::Format(wxT("%s x%i"), shipName, count);
        addInfo(infoGrid, (x == 0 ? "Ships:" : ""), wxShipName);
      }
    }
  }
  infoSizer_->Add(infoGrid);

  char *description = fileManager_->getStageDescription(
      getStagesDir().c_str(), stageName, getCacheDir().c_str());
  if (description == 0) {
    std::string descstr("<No description>");
    description = new char[descstr.length() + 1];
    strcpy(description, descstr.c_str());
  }
  wxStaticText *descCtrl = new wxStaticText(mainPanel_, wxID_ANY, description);
  descSizer_->Add(descCtrl);
  delete description;

  mainPanel_->GetSizer()->SetSizeHints(mainPanel_);
  mainPanel_->Layout();
  Fit();
  mainPanel_->SetFocus();

  // On Windows, if we set the bitmap before the Layout/Fit stuff on Windows, we
  // get visual artifacts when paging through the stages with up/down keys.
  visualPreview_->SetBitmap(wxBitmap(previewImage));
  delete engine;
}

void StagePreview::addInfo(wxSizer *sizer, const char *name,
                           const char *value) {
  if (name != 0) {
    sizer->Add(new wxStaticText(mainPanel_, wxID_ANY, name));
  }
  sizer->Add(new wxStaticText(mainPanel_, wxID_ANY, value));
}

void StagePreview::addInfo(wxSizer *sizer, const char *name, int i) {
  if (i > 0) {
    sizer->Add(new wxStaticText(mainPanel_, wxID_ANY, name));
    sizer->Add(new wxStaticText(mainPanel_, wxID_ANY,
                                wxString::Format(wxT("%i"), i)));
  }
}

char* StagePreview::savePreviewImage(sf::RenderWindow *window,
    BerryBotsEngine *engine, unsigned int &targetWidth,
    unsigned int &targetHeight) {
  Stage *stage = engine->getStage();
  unsigned int viewWidth = stage->getWidth() + (2 * STAGE_MARGIN);
  unsigned int viewHeight = stage->getHeight() + (2 * STAGE_MARGIN);
  unsigned int screenWidth = MAX_PREVIEW_WIDTH;
  unsigned int screenHeight = MAX_PREVIEW_HEIGHT;
  double windowScale =
      std::min(1.0, std::min(((double) screenWidth) / viewWidth,
                             ((double) screenHeight) / viewHeight));
  targetWidth = round(windowScale * viewWidth);
  targetHeight = round(windowScale * viewHeight);

#ifdef __WXGTK__
  // Since setSize() doesn't work reliably, we create it inline on Linux.
  window = new sf::RenderWindow(
      sf::VideoMode(targetWidth, targetHeight), "Preview",
      sf::Style::None,
      sf::ContextSettings(0, 0, (isAaDisabled() ? 0 : 4), 2, 0));
  window->setVisible(false);
#else
  window->setSize(sf::Vector2u(targetWidth, targetHeight));
#endif

  Team **teams = new Team*[1];
  teams[0] = new Team;
  strcpy(teams[0]->name, "PreviewTeam");
  teams[0]->numRectangles = 0;
  teams[0]->numLines = 0;
  teams[0]->numCircles = 0;
  teams[0]->numTexts = 0;
  Ship **ships = new Ship*[1];
  Ship *ship = new Ship;
  ShipProperties *properties = new ShipProperties;
  properties->shipR = properties->shipG = properties->shipB = 255;
  properties->laserR = properties->laserB = 0;
  properties->laserG = 255;
  properties->thrusterR = 255;
  properties->thrusterG = properties->thrusterB = 0;
  strcpy(properties->name, "PreviewShip");
  ship->properties = properties;
  ship->thrusterAngle = ship->thrusterForce = 0;
  Point2D *start = stage->getStart();
  ship->x = start->getX();
  ship->y = start->getY();
  ship->alive = true;
  ship->showName = ship->energyEnabled = false;
  ships[0] = ship;
  teams[0]->numTexts = 0;
  stage->setTeamsAndShips(teams, 1, ships, 1);

  previewGfxManager_->initBbGfx(
      window, 1, viewHeight, stage, teams, 1, ships, 1);
  previewGfxManager_->initViews(window, viewWidth, viewHeight);

  GfxEventHandler *gfxHandler = new GfxEventHandler();
  window->clear();
  previewGfxManager_->drawGame(window, stage, ships, 1, 0, gfxHandler, false,
                               false, 0);

  std::stringstream filenameStream;
  filenameStream << (rand() % 10000000) << ".png";
  char *filename = fileManager_->getFilePath(getTmpDir().c_str(),
                                             filenameStream.str().c_str());
  char *absFilename = fileManager_->getAbsFilePath(filename);
  delete filename;

  sf::Image previewImage = window->capture();
  fileManager_->createDirectoryIfNecessary(getTmpDir().c_str());
  previewImage.saveToFile(absFilename);
#ifdef __WXGTK__
  delete window;
#endif
  previewGfxManager_->destroyBbGfx();

  delete gfxHandler;
  delete properties;
  delete teams[0];
  delete teams;

  return absFilename;
}

PreviewEventFilter::PreviewEventFilter(StagePreview *stagePreview) {
  stagePreview_ = stagePreview;
}

int PreviewEventFilter::FilterEvent(wxEvent& event) {
  if (stagePreview_->IsActive()) {
    const wxEventType type = event.GetEventType();
    if (type == wxEVT_KEY_DOWN && stagePreview_->IsActive()) {
      wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
      int keyCode = keyEvent->GetKeyCode();
      if (keyCode == WXK_ESCAPE || keyCode == WXK_SPACE
          || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
        stagePreview_->Close();
        return Event_Processed;
      } else if (keyCode == WXK_UP) {
        stagePreview_->onUp();
        return Event_Processed;
      } else if (keyCode == WXK_DOWN) {
        stagePreview_->onDown();
        return Event_Processed;
      }
    } else if (type == wxEVT_NAVIGATION_KEY) {
      wxNavigationKeyEvent *keyEvent = ((wxNavigationKeyEvent*) &event);
      if (keyEvent->GetDirection()) {
        stagePreview_->onDown();
      } else {
        stagePreview_->onUp();
      }
      return Event_Processed;
    }
  }

  return Event_Skip;
}
