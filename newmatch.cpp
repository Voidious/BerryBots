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

NewMatchDialog::NewMatchDialog(NewMatchListener *listener) : wxFrame(NULL,
    NEW_MATCH_ID, "New Match", wxPoint(50, 50), wxDefaultSize,
    wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  borderSizer_ = new wxBoxSizer(wxHORIZONTAL);
  gridSizer_ = new wxFlexGridSizer(3, 5, 5);
  stageSizer_ = new wxBoxSizer(wxVERTICAL);
  botsSizer_ = new wxBoxSizer(wxVERTICAL);
  botButtonsSizer_ = new wxBoxSizer(wxVERTICAL);
  loadedBotsSizer_ = new wxBoxSizer(wxVERTICAL);

  stageLabel_ = new wxStaticText(this, -1, "Stage:");
  stageSelect_ = new wxListBox(this, -1, wxDefaultPosition, wxSize(275, 225), 0,
                               NULL, wxLB_SORT);
  stageSizer_->Add(stageLabel_, 0, wxALIGN_LEFT);
  stageSizer_->AddSpacer(3);
  stageSizer_->Add(stageSelect_, 0, wxALIGN_LEFT);
  gridSizer_->Add(stageSizer_, 0, wxALIGN_LEFT);
  gridSizer_->AddSpacer(0);
  gridSizer_->AddSpacer(0);

  botsLabel_ = new wxStaticText(this, -1, "Ships:");
  botsSelect_ = new wxListBox(this, SELECT_BOTS_ID, wxDefaultPosition,
                              wxSize(275, 225), 0, NULL,
                              wxLB_MULTIPLE | wxLB_SORT);
  botsSizer_->Add(botsLabel_, 0, wxALIGN_LEFT);
  botsSizer_->AddSpacer(3);
  botsSizer_->Add(botsSelect_, 0, wxALIGN_LEFT);
  gridSizer_->Add(botsSizer_, 0, wxALIGN_LEFT);

  addArrow_ = new wxButton(this, ADD_BUTTON_ID, ">>", wxDefaultPosition,
                           wxDefaultSize, wxBU_EXACTFIT);
  removeArrow_ = new wxButton(this, REMOVE_BUTTON_ID, "<<", wxDefaultPosition,
                              wxDefaultSize, wxBU_EXACTFIT);
  clearButton_ = new wxButton(this, CLEAR_BUTTON_ID, "Clear", wxDefaultPosition,
                              wxDefaultSize, wxBU_EXACTFIT);
  botButtonsSizer_->Add(addArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer_->AddSpacer(5);
  botButtonsSizer_->Add(removeArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer_->AddSpacer(5);
  botButtonsSizer_->Add(clearButton_, 0, wxALIGN_CENTER);
  gridSizer_->Add(botButtonsSizer_, 0, wxALIGN_CENTER);

  loadedBotsSelect_ = new wxListBox(this, LOADED_BOTS_ID, wxDefaultPosition,
                                    wxSize(275, 225), 0, NULL, wxLB_MULTIPLE);
  gridSizer_->Add(loadedBotsSelect_, 0, wxALIGN_BOTTOM | wxALIGN_RIGHT);

  refreshButton_ = new wxButton(this, wxID_REFRESH);
  gridSizer_->Add(refreshButton_, 0, wxALIGN_LEFT);
  gridSizer_->AddSpacer(0);
  startButton_ = new wxButton(this, START_BUTTON_ID, "Start Match!",
                              wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  gridSizer_->Add(startButton_, 0, wxALIGN_RIGHT);
  borderSizer_->Add(gridSizer_, 0, wxALL, 12);
  SetSizerAndFit(borderSizer_);

  numStages_ = numBots_ = numLoadedBots_ = 0;
  menusInitialized_ = false;

  Connect(NEW_MATCH_ID, wxEVT_ACTIVATE,
          wxActivateEventHandler(NewMatchDialog::onActivate));
  Connect(NEW_MATCH_ID, wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(NewMatchDialog::onClose));
  Connect(ADD_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddBots));
  Connect(REMOVE_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveBots));
  Connect(CLEAR_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onClearLoadedBots));
  Connect(START_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onStartMatch));
  Connect(SELECT_BOTS_ID, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddBots));
  Connect(LOADED_BOTS_ID, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveBots));
  Connect(wxID_REFRESH, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onRefreshFiles));
  
  eventFilter_ = new NewMatchEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

NewMatchDialog::~NewMatchDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete listener_;
  delete stageLabel_;
  delete stageSelect_;
  delete botsLabel_;
  delete botsSelect_;
  delete addArrow_;
  delete removeArrow_;
  delete clearButton_;
  delete loadedBotsSelect_;
  delete startButton_;
  delete refreshButton_;
}

void NewMatchDialog::clearStages() {
  stageSelect_->Clear();
  numStages_ = 0;
}

void NewMatchDialog::addStage(char *stage) {
  stageSelect_->Append(wxString(stage));
  numStages_++;
  if (stageSelect_->GetCount() > 0) {
    stageSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::clearBots() {
  botsSelect_->Clear();
  numBots_ = 0;
}

void NewMatchDialog::addBot(char *bot) {
  botsSelect_->Append(wxString(bot));
  numBots_++;
  if (botsSelect_->GetCount() > 0) {
    botsSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::onActivate(wxActivateEvent &event) {
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

void NewMatchDialog::removeStaleLoadedBots() {
  wxArrayInt loadedBots;
  if (numLoadedBots_ != 0) {
    for (int x = 0; x < numLoadedBots_; x++) {
      wxString loadedBot = loadedBotsSelect_->GetString(x);
      if (botsSelect_->FindString(loadedBot) == wxNOT_FOUND) {
        loadedBotsSelect_->Delete(x);
        x--;
        numLoadedBots_--;
      }
    }
  }
}

void NewMatchDialog::onClearLoadedBots(wxCommandEvent &event) {
  loadedBotsSelect_->Clear();
  numLoadedBots_ = 0;
}

void NewMatchDialog::onStartMatch(wxCommandEvent &event) {
  startMatch();
}

void NewMatchDialog::startMatch() {
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

void NewMatchDialog::onRefreshFiles(wxCommandEvent &event) {
  listener_->refreshFiles();
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
  if (type == wxEVT_KEY_DOWN && newMatchDialog_->IsActive()) {
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE) {
      newMatchDialog_->onEscape();
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_RETURN)
               && newMatchDialog_->botsSelectHasFocus()) {
      newMatchDialog_->addSelectedBots();
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_BACK)
               && (newMatchDialog_->loadedBotsSelectHasFocus())) {
      newMatchDialog_->removeSelectedLoadedBots();
    } else if (keyEvent->GetUnicodeKey() == 'M' && keyEvent->ControlDown()) {
      newMatchDialog_->startMatch();
    }
#ifdef __WXOSX__
    if (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown()) {
      newMatchDialog_->onEscape();
    }
#endif
  }
  return Event_Skip;
}
