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
#include "bbconst.h"

NewMatchDialog::NewMatchDialog() : wxFrame(NULL, NEW_MATCH_ID, "New Match...",
    wxPoint(50, 50), wxSize(500, 520),
    wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  stageLabel = new wxStaticText(this, -1, "Stage:", wxPoint(20, 10));
  stageSelect = new wxListBox(this, -1, wxPoint(20, 30), wxSize(200, 200));
  botsLabel = new wxStaticText(this, -1, "Bots:", wxPoint(20, 240));
  botsSelect = new wxListBox(this, SELECT_BOTS_ID, wxPoint(20, 260),
                             wxSize(200, 200), 0, NULL, wxLB_MULTIPLE);
  addArrow = new wxButton(this, ADD_BUTTON_ID, ">>", wxPoint(224, 315),
                          wxDefaultSize, wxBU_EXACTFIT);
  removeArrow = new wxButton(this, REMOVE_BUTTON_ID, "<<", wxPoint(224, 345),
                          wxDefaultSize, wxBU_EXACTFIT);
  clearButton = new wxButton(this, CLEAR_BUTTON_ID, "Clear", wxPoint(217, 375),
                             wxDefaultSize, wxBU_EXACTFIT);
  loadedBotsSelect = new wxListBox(this, LOADED_BOTS_ID, wxPoint(280, 260),
                                   wxSize(200, 200), 0, NULL, wxLB_MULTIPLE);
  startButton = new wxButton(this, START_BUTTON_ID, "Start Match!",
                             wxPoint(377, 460), wxDefaultSize, wxBU_EXACTFIT);
  numStages_ = numBots_ = numLoadedBots_ = 0;
  listener_ = 0;

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
  Connect(NEW_MATCH_ID, wxEVT_ACTIVATE,
          wxCommandEventHandler(NewMatchDialog::onActivate));
}

NewMatchDialog::~NewMatchDialog() {
  delete stageLabel;
  delete stageSelect;
  delete botsLabel;
  delete botsSelect;
  delete addArrow;
  delete clearButton;
  delete loadedBotsSelect;
  delete startButton;
}

void NewMatchDialog::addStage(char *stage) {
  stageSelect->Insert(wxString(stage), numStages_++);
}

void NewMatchDialog::addBot(char *bot) {
  botsSelect->Insert(wxString(bot), numBots_++);
}

void NewMatchDialog::resetCursors() {
  if (stageSelect->GetCount() > 0) {
    stageSelect->SetFirstItem(0);
  }
  if (botsSelect->GetCount() > 0) {
    botsSelect->SetFirstItem(0);
  }
}

void NewMatchDialog::onAddBots(wxCommandEvent &event) {
  wxArrayInt selectedBots;
  botsSelect->GetSelections(selectedBots);
  wxArrayInt::const_iterator first = selectedBots.begin();
  wxArrayInt::const_iterator last = selectedBots.end();
  while (first != last) {
    int botIndex = *first++;
    loadedBotsSelect->Insert(botsSelect->GetString(botIndex),
                             numLoadedBots_++);
  }
}

void NewMatchDialog::onRemoveBots(wxCommandEvent &event) {
  wxArrayInt selectedBots;
  loadedBotsSelect->GetSelections(selectedBots);
  wxArrayInt::const_iterator first = selectedBots.begin();
  wxArrayInt::const_iterator last = selectedBots.end();
  int removed = 0;
  while (first != last) {
    int botIndex = *first++;
    loadedBotsSelect->Delete(botIndex - (removed++));
    numLoadedBots_--;
  }
}

void NewMatchDialog::onClearBots(wxCommandEvent &event) {
  loadedBotsSelect->Clear();
  numLoadedBots_ = 0;
}

void NewMatchDialog::onStartMatch(wxCommandEvent &event) {
  if (listener_ != 0) {
    wxArrayInt loadedBots;
    wxArrayInt selectedStageIndex;
    stageSelect->GetSelections(selectedStageIndex);
    if (numLoadedBots_ != 0 && selectedStageIndex.Count() != 0) {
      const char** bots = new const char*[numLoadedBots_];
      for (int x = 0; x < numLoadedBots_; x++) {
        wxString loadedBot = loadedBotsSelect->GetString(x);
        char *bot = new char[loadedBot.length() + 1];
        strcpy(bot, loadedBot.fn_str());
        bots[x] = bot;
      }

      wxString selectedStage =
          stageSelect->GetString(*(selectedStageIndex.begin()));
      char *stage = new char[selectedStage.length() + 1];
      strcpy(stage, selectedStage.fn_str());

      listener_->startMatch(stage, bots, numLoadedBots_);
    }
  }
}

void NewMatchDialog::onClose(wxCommandEvent &event) {
  listener_->cancel();
}

void NewMatchDialog::onActivate(wxCommandEvent &event) {
  this->initMenus();
}

void NewMatchDialog::setListener(NewMatchListener *listener) {
  listener_ = listener;
}

void NewMatchDialog::initMenus() {
  wxMenu *fileMenu = new wxMenu();
  fileMenu->Insert(0, NEW_MATCH_MENU_ID, "New Match...", 0);
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Insert(0, fileMenu, "File");

  this->SetMenuBar(menuBar);
}
