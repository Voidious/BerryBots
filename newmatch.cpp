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
#include "newmatch.h"
#include "bbwx.h"

NewMatchDialog::NewMatchDialog() : wxFrame(NULL, NEW_MATCH_ID, "BerryBots",
    wxPoint(50, 50), wxSize(500, 520),
    wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  stageLabel_ = new wxStaticText(this, -1, "Stage:", wxPoint(20, 10));
  stageSelect_ = new wxListBox(this, -1, wxPoint(20, 30), wxSize(200, 200));
  botsLabel_ = new wxStaticText(this, -1, "Bots:", wxPoint(20, 240));
  botsSelect_ = new wxListBox(this, SELECT_BOTS_ID, wxPoint(20, 260),
                              wxSize(200, 200), 0, NULL, wxLB_MULTIPLE);
  addArrow_ = new wxButton(this, ADD_BUTTON_ID, ">>", wxPoint(224, 315),
                           wxDefaultSize, wxBU_EXACTFIT);
  removeArrow_ = new wxButton(this, REMOVE_BUTTON_ID, "<<", wxPoint(224, 345),
                              wxDefaultSize, wxBU_EXACTFIT);
  clearButton_ = new wxButton(this, CLEAR_BUTTON_ID, "Clear", wxPoint(217, 375),
                              wxDefaultSize, wxBU_EXACTFIT);
  loadedBotsSelect_ = new wxListBox(this, LOADED_BOTS_ID, wxPoint(280, 260),
                                    wxSize(200, 200), 0, NULL, wxLB_MULTIPLE);
  startButton_ = new wxButton(this, START_BUTTON_ID, "Start Match!",
                              wxPoint(377, 460), wxDefaultSize, wxBU_EXACTFIT);
  numStages_ = numBots_ = numLoadedBots_ = 0;
  listener_ = 0;
  menusInitialized_ = false;

  Connect(NEW_MATCH_ID, wxEVT_ACTIVATE,
          wxCommandEventHandler(NewMatchDialog::onActivate));
  Connect(NEW_MATCH_ID, wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(NewMatchDialog::onClose));
  Connect(ADD_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddBots));
  Connect(REMOVE_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveBots));
  Connect(CLEAR_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onClearBots));
  Connect(START_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onStartMatch));
  Connect(SELECT_BOTS_ID, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddBots));
  Connect(LOADED_BOTS_ID, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveBots));

  this->GetEventHandler()->AddFilter(new NewMatchEventFilter(this));
}

NewMatchDialog::~NewMatchDialog() {
  delete stageLabel_;
  delete stageSelect_;
  delete botsLabel_;
  delete botsSelect_;
  delete addArrow_;
  delete removeArrow_;
  delete clearButton_;
  delete loadedBotsSelect_;
  delete startButton_;
}

void NewMatchDialog::setListener(NewMatchListener *listener) {
  listener_ = listener;
}

void NewMatchDialog::addStage(char *stage) {
  stageSelect_->Insert(wxString(stage), numStages_++);
  if (stageSelect_->GetCount() > 0) {
    stageSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::addBot(char *bot) {
  botsSelect_->Insert(wxString(bot), numBots_++);
  if (botsSelect_->GetCount() > 0) {
    botsSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::onActivate(wxCommandEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(listener_->getNewMenuBar());
    menusInitialized_ = true;
  }
}

void NewMatchDialog::onClose(wxCommandEvent &event) {
  listener_->cancel();
}

void NewMatchDialog::onAddBots(wxCommandEvent &event) {
  addSelectedBots();
}

void NewMatchDialog::addSelectedBots() {
  wxArrayInt selectedBots;
  botsSelect_->GetSelections(selectedBots);
  wxArrayInt::const_iterator first = selectedBots.begin();
  wxArrayInt::const_iterator last = selectedBots.end();
  while (first != last) {
    int botIndex = *first++;
    loadedBotsSelect_->Insert(botsSelect_->GetString(botIndex),
                              numLoadedBots_++);
  }
}

void NewMatchDialog::onRemoveBots(wxCommandEvent &event) {
  removeSelectedLoadedBots();
}

void NewMatchDialog::removeSelectedLoadedBots() {
  wxArrayInt selectedBots;
  loadedBotsSelect_->GetSelections(selectedBots);
  wxArrayInt::const_iterator first = selectedBots.begin();
  wxArrayInt::const_iterator last = selectedBots.end();
  int removed = 0;
  while (first != last) {
    int botIndex = *first++;
    loadedBotsSelect_->Delete(botIndex - (removed++));
    numLoadedBots_--;
  }
}

void NewMatchDialog::onClearBots(wxCommandEvent &event) {
  loadedBotsSelect_->Clear();
  numLoadedBots_ = 0;
}

void NewMatchDialog::onStartMatch(wxCommandEvent &event) {
  if (listener_ != 0) {
    wxArrayInt loadedBots;
    wxArrayInt selectedStageIndex;
    stageSelect_->GetSelections(selectedStageIndex);
    if (numLoadedBots_ != 0 && selectedStageIndex.Count() != 0) {
      int numStartBots = numLoadedBots_;
      char** bots = new char*[numStartBots];
      for (int x = 0; x < numStartBots; x++) {
        wxString loadedBot = loadedBotsSelect_->GetString(x);
        char *bot = new char[loadedBot.length() + 1];
        strcpy(bot, loadedBot.fn_str());
        bots[x] = bot;
      }

      wxString selectedStage =
          stageSelect_->GetString(*(selectedStageIndex.begin()));
      char *stage = new char[selectedStage.length() + 1];
      strcpy(stage, selectedStage.fn_str());

      listener_->startMatch(stage, bots, numStartBots);

      for (int x = 0; x < numStartBots; x++) {
        delete bots[x];
      }
      delete bots;
      delete stage;
    }
  }
}

void NewMatchDialog::onEscape() {
  listener_->cancel();
}

bool NewMatchDialog::botsSelectHasFocus() {
  return botsSelect_->HasFocus();
}

bool NewMatchDialog::loadedBotsSelectHasFocus() {
  return loadedBotsSelect_->HasFocus();
}

NewMatchEventFilter::NewMatchEventFilter(NewMatchDialog *newMatchDialog) {
  newMatchDialog_ = newMatchDialog;
}

NewMatchEventFilter::~NewMatchEventFilter() {
  
}

int NewMatchEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN) {
    int keyCode = ((wxKeyEvent*) &event)->GetKeyCode();
    if (keyCode == WXK_ESCAPE) {
      newMatchDialog_->onEscape();
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_RETURN)
               && newMatchDialog_->botsSelectHasFocus()) {
      newMatchDialog_->addSelectedBots();
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_BACK)
               && (newMatchDialog_->loadedBotsSelectHasFocus())) {
      newMatchDialog_->removeSelectedLoadedBots();
    }
  }
  return Event_Skip;
}
