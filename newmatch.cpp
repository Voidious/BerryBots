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

NewMatchDialog::NewMatchDialog(NewMatchListener *listener,
    MenuBarMaker *menuBarMaker) : wxFrame(NULL, NEW_MATCH_ID, "New Match",
        wxPoint(50, 50), wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  menuBarMaker_ = menuBarMaker;
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
                              wxLB_EXTENDED | wxLB_SORT);
  botsSizer_->Add(botsLabel_, 0, wxALIGN_LEFT);
  botsSizer_->AddSpacer(3);
  botsSizer_->Add(botsSelect_, 0, wxALIGN_LEFT);
  gridSizer_->Add(botsSizer_, 0, wxALIGN_LEFT);

  addArrow_ = new wxButton(this, ADD_BUTTON_ID, ">>", wxDefaultPosition,
                           wxDefaultSize);
  removeArrow_ = new wxButton(this, REMOVE_BUTTON_ID, "<<", wxDefaultPosition,
                              wxDefaultSize);
  clearButton_ = new wxButton(this, CLEAR_BUTTON_ID, "&Clear", wxDefaultPosition,
                              wxDefaultSize);
  botButtonsSizer_->Add(addArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer_->AddSpacer(5);
  botButtonsSizer_->Add(removeArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer_->AddSpacer(5);
  botButtonsSizer_->Add(clearButton_, 0, wxALIGN_CENTER);
  gridSizer_->Add(botButtonsSizer_, 0, wxALIGN_CENTER);

  loadedBotsSelect_ = new wxListBox(this, LOADED_BOTS_ID, wxDefaultPosition,
                                    wxSize(275, 225), 0, NULL, wxLB_EXTENDED);
  gridSizer_->Add(loadedBotsSelect_, 0, wxALIGN_BOTTOM | wxALIGN_RIGHT);

  refreshButton_ = new wxButton(this, wxID_REFRESH, "    &Refresh    ");
  gridSizer_->Add(refreshButton_, 0, wxALIGN_LEFT);
  gridSizer_->AddSpacer(0);
  startButton_ = new wxButton(this, START_BUTTON_ID, "    Start &Match!    ",
                              wxDefaultPosition, wxDefaultSize);
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
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
  }
  SetSizerAndFit(borderSizer_);
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
      if (botsSelect_->wxItemContainerImmutable::FindString(loadedBot)
              == wxNOT_FOUND) {
        loadedBotsSelect_->Delete(x);
        x--;
        numLoadedBots_--;
      }
    }
  }
}

void NewMatchDialog::onClearLoadedBots(wxCommandEvent &event) {
  clearLoadedBots();
}

void NewMatchDialog::clearLoadedBots() {
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
#ifdef __WINDOWS__
        strcpy(bot, loadedBot.c_str());
#else
        strcpy(bot, loadedBot.fn_str());
#endif
        bots[x] = bot;
      }
      
      wxString selectedStage =
          stageSelect_->GetString(*(selectedStageIndex.begin()));
      char *stage = new char[selectedStage.length() + 1];
#ifdef __WINDOWS__
      strcpy(stage, selectedStage.c_str());
#else
      strcpy(stage, selectedStage.fn_str());
#endif
      
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
  refreshFiles();
}

void NewMatchDialog::refreshFiles() {
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

void NewMatchDialog::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    clearButton_->SetLabel("&Clear \u2318C");
    refreshButton_->SetLabel("&Refresh \u2318R");
    startButton_->SetLabel("Start &Match \u2318M");
#else
    clearButton_->SetLabel("&Clear  alt-C");
    refreshButton_->SetLabel("&Refresh  alt-R");
    startButton_->SetLabel("Start &Match!  alt-M");
#endif
  } else {
    clearButton_->SetLabel("&Clear");
    refreshButton_->SetLabel("    &Refresh    ");
    startButton_->SetLabel("    Start &Match!    ");
  }
}

NewMatchEventFilter::NewMatchEventFilter(NewMatchDialog *newMatchDialog) {
  newMatchDialog_ = newMatchDialog;
}

NewMatchEventFilter::~NewMatchEventFilter() {

}

int NewMatchEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && newMatchDialog_->IsActive()) {
    newMatchDialog_->setMnemonicLabels(modifierDown);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      newMatchDialog_->onEscape();
      return Event_Processed;
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_RETURN)
               && newMatchDialog_->botsSelectHasFocus()) {
      newMatchDialog_->addSelectedBots();
      return Event_Processed;
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_BACK)
               && (newMatchDialog_->loadedBotsSelectHasFocus())) {
      newMatchDialog_->removeSelectedLoadedBots();
#if defined(__WXOSX__) || defined(__WINDOWS__)
    // Mac OS X doesn't handle mnemonics, so add some manual keyboard shortcuts.
    // Tab navigation and mnemonics are also broken for me on Windows 8 right
    // now, though I think they're supposd to work. So enable them there too.
    } else if (keyEvent->GetUnicodeKey() == 'M' && modifierDown) {
      newMatchDialog_->startMatch();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'R' && modifierDown) {
      newMatchDialog_->refreshFiles();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'C' && modifierDown) {
      newMatchDialog_->clearLoadedBots();
      return Event_Processed;
#endif
    }
  }
  if (type == wxEVT_KEY_UP) {
    newMatchDialog_->setMnemonicLabels(modifierDown);
  }
  // TODO: Do we need to handle tab navigation manually on Windows? Nothing
  //       working at all on Windows 8 for me right now.
  return Event_Skip;
}
