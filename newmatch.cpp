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
#include <wx/artprov.h>
#include "bbwx.h"
#include "basedir.h"
#include "newmatch.h"

NewMatchDialog::NewMatchDialog(NewMatchListener *listener,
    MenuBarMaker *menuBarMaker) : wxFrame(NULL, NEW_MATCH_ID, "New Match",
        wxPoint(50, 50), wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  menuBarMaker_ = menuBarMaker;

  mainPanel_ = new wxPanel(this);
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  borderSizer_ = new wxBoxSizer(wxHORIZONTAL);
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 5, 5);
  wxBoxSizer *stageSizer = new wxBoxSizer(wxVERTICAL);
  stageLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "Stage:");
  stageSelect_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                               wxSize(275, 225), 0, NULL, wxLB_SORT);
  stageSizer->Add(stageLabel_, 0, wxALIGN_LEFT);
  stageSizer->AddSpacer(3);
  stageSizer->Add(stageSelect_, 0, wxALIGN_LEFT);
  gridSizer->Add(stageSizer, 0, wxALIGN_LEFT);

  wxBoxSizer *dirsBorderSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *dirsSizer = new wxBoxSizer(wxVERTICAL);
  stageBaseDirLabel_ = new wxStaticText(mainPanel_, wxID_ANY, wxEmptyString);
  botsBaseDirLabel_ = new wxStaticText(mainPanel_, wxID_ANY, wxEmptyString);
  updateBaseDirLabels();
  browseStagesButton_ = new wxButton(mainPanel_, wxID_ANY, "Open");
  browseStagesButton_->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN));
  browseShipsButton_ = new wxButton(mainPanel_, wxID_ANY, "Open");
  browseShipsButton_->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN));
  dirsSizer->Add(stageBaseDirLabel_);
  dirsSizer->AddSpacer(3);
  dirsSizer->Add(browseStagesButton_);
  dirsSizer->AddSpacer(12);
  dirsSizer->Add(botsBaseDirLabel_);
  dirsSizer->AddSpacer(3);
  dirsSizer->Add(browseShipsButton_);

#ifdef __WXOSX__
  // Using cwd as base dir on other platforms, so only support changing base dir
  // on Mac for now.
  folderButton_ = new wxButton(mainPanel_, wxID_ANY, "Change &Base Dir  ");
  folderButton_->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER));
  dirsSizer->AddStretchSpacer(1);
  dirsSizer->Add(folderButton_, 0, wxALIGN_BOTTOM);

  Connect(folderButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onChangeBaseDir));
#else
  folderButton_ = 0;
#endif
  dirsBorderSizer->AddSpacer(12);
  dirsBorderSizer->Add(dirsSizer, 0, wxEXPAND);
  gridSizer->Add(dirsBorderSizer, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP);

  botsLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "Ships:");
  botsSelect_ = new wxListBox(mainPanel_, SELECT_BOTS_ID, wxDefaultPosition,
                              wxSize(275, 225), 0, NULL,
                              wxLB_EXTENDED | wxLB_SORT);
  wxBoxSizer *botsSizer = new wxBoxSizer(wxVERTICAL);
  botsSizer->Add(botsLabel_, 0, wxALIGN_LEFT);
  botsSizer->AddSpacer(3);
  botsSizer->Add(botsSelect_, 0, wxALIGN_LEFT);
  gridSizer->Add(botsSizer, 0, wxALIGN_LEFT);

  wxBoxSizer *buttonsLoadedBotsSizer = new wxBoxSizer(wxHORIZONTAL);
  addArrow_ = new wxButton(mainPanel_, ADD_BUTTON_ID, ">>",
                           wxDefaultPosition, wxDefaultSize);
  removeArrow_ = new wxButton(mainPanel_, REMOVE_BUTTON_ID, "<<",
                              wxDefaultPosition, wxDefaultSize);
  clearButton_ = new wxButton(mainPanel_, CLEAR_BUTTON_ID, "C&lear",
                              wxDefaultPosition, wxDefaultSize);

  wxBoxSizer *botButtonsSizer = new wxBoxSizer(wxVERTICAL);
  botButtonsSizer->AddStretchSpacer(1);
  botButtonsSizer->Add(addArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer->AddSpacer(5);
  botButtonsSizer->Add(removeArrow_, 0, wxALIGN_CENTER);
  botButtonsSizer->AddSpacer(5);
  botButtonsSizer->Add(clearButton_, 0, wxALIGN_CENTER);
  botButtonsSizer->AddStretchSpacer(1);
#ifdef __WXOSX__
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                    "\u2318 hotkeys");
#else
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                    "ALT hotkeys");
#endif
  botButtonsSizer->Add(keyboardLabel_, 0, wxALIGN_CENTER | wxALIGN_BOTTOM);
  buttonsLoadedBotsSizer->Add(botButtonsSizer, 0, wxALIGN_CENTER | wxEXPAND);

  loadedBotsSelect_ = new wxListBox(mainPanel_, LOADED_BOTS_ID,
      wxDefaultPosition, wxSize(275, 225), 0, NULL, wxLB_EXTENDED);
  buttonsLoadedBotsSizer->AddSpacer(5);
  buttonsLoadedBotsSizer->Add(loadedBotsSelect_, 0,
                              wxALIGN_BOTTOM | wxALIGN_RIGHT);
  gridSizer->Add(buttonsLoadedBotsSizer, 0, wxALIGN_BOTTOM);

  refreshButton_ = new wxButton(mainPanel_, wxID_REFRESH, "    &Refresh    ");
  gridSizer->Add(refreshButton_, 0, wxALIGN_LEFT);

  startButton_ = new wxButton(mainPanel_, START_BUTTON_ID,
      "    Start &Match!    ", wxDefaultPosition, wxDefaultSize);
  gridSizer->Add(startButton_, 0, wxALIGN_RIGHT);
  borderSizer_->Add(gridSizer, 0, wxALL, 12);
  mainPanel_->SetSizerAndFit(borderSizer_);
  SetSizerAndFit(mainSizer_);

  browseStagesButton_->MoveAfterInTabOrder(startButton_);
  browseShipsButton_->MoveAfterInTabOrder(browseStagesButton_);
#ifdef __WXOSX__
  folderButton_->MoveAfterInTabOrder(browseShipsButton_);
#endif

  numStages_ = numBots_ = numLoadedBots_ = 0;
  menusInitialized_ = false;

  Connect(NEW_MATCH_ID, wxEVT_ACTIVATE,
          wxActivateEventHandler(NewMatchDialog::onActivate));
  Connect(NEW_MATCH_ID, wxEVT_SHOW, wxShowEventHandler(NewMatchDialog::onShow));
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
  Connect(browseStagesButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onBrowseStages));
  Connect(browseShipsButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onBrowseShips));
  
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
  if (folderButton_ != 0) {
    delete folderButton_;
  }
  delete addArrow_;
  delete removeArrow_;
  delete clearButton_;
  delete loadedBotsSelect_;
  delete startButton_;
  delete refreshButton_;
  delete stageBaseDirLabel_;
  delete browseStagesButton_;
  delete botsBaseDirLabel_;
  delete browseShipsButton_;
  delete keyboardLabel_;
  delete mainPanel_;
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
  SetSizerAndFit(mainSizer_);
}

void NewMatchDialog::onShow(wxShowEvent &event) {
  initialFocus();
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

void NewMatchDialog::onBrowseStages(wxCommandEvent &event) {
  openFile(getStageDir().c_str());
}

void NewMatchDialog::onBrowseShips(wxCommandEvent &event) {
  openFile(getBotsDir().c_str());
}

void NewMatchDialog::openFile(const char *file) {
#if defined(__WXOSX__)
  ::wxExecute(wxString::Format("open %s", file), wxEXEC_ASYNC, NULL);
#elif defined(__LINUX__)
  ::wxExecute(wxString::Format("nautilus %s", file), wxEXEC_ASYNC, NULL);
#elif defined(__WINDOWS__)
  ::wxExecute(wxString::Format("explorer %s", file), wxEXEC_ASYNC, NULL);
#else
  wxMessageDialog cantBrowseMessage(this,
      "Sorry, don't know how to open/browse files on your platform.", wxOK);
  cantBrowseMessage.ShowModal();
#endif
}

void NewMatchDialog::onChangeBaseDir(wxCommandEvent &event) {
  changeBaseDir();
}

void NewMatchDialog::changeBaseDir() {
  chooseNewRootDir();
  updateBaseDirLabels();
  listener_->reloadBaseDirs();
}

void NewMatchDialog::updateBaseDirLabels() {
  wxString stageBaseDirLabelText("Stages:  ");
  stageBaseDirLabelText.Append(getStageDir());
  stageBaseDirLabel_->SetLabelText(stageBaseDirLabelText);
  wxString botsBaseDirLabelText("Ships:  ");
  botsBaseDirLabelText.Append(getBotsDir());
  botsBaseDirLabel_->SetLabelText(botsBaseDirLabelText);
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
    folderButton_->SetLabel("Change &Base \u2318B");
    clearButton_->SetLabel("C&lear \u2318L");
    refreshButton_->SetLabel("&Refresh \u2318R");
    startButton_->SetLabel("Start &Match \u2318M");
#else
    clearButton_->SetLabel("C&lear  alt-L");
    refreshButton_->SetLabel("&Refresh  alt-R");
    startButton_->SetLabel("Start &Match!  alt-M");
#endif
  } else {
#ifdef __WXOSX__
    folderButton_->SetLabel("Change &Base Dir  ");
#endif
    clearButton_->SetLabel("C&lear");
    refreshButton_->SetLabel("    &Refresh    ");
    startButton_->SetLabel("    Start &Match!    ");
  }
}

void NewMatchDialog::initialFocus() {
  stageSelect_->SetFocus();
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
#ifdef __WXOSX__
    // Mac OS X doesn't handle mnemonics, so add some manual keyboard shortcuts.
    } else if (keyEvent->GetUnicodeKey() == 'M' && modifierDown) {
      newMatchDialog_->startMatch();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'R' && modifierDown) {
      newMatchDialog_->refreshFiles();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'L' && modifierDown) {
      newMatchDialog_->clearLoadedBots();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'B' && modifierDown) {
      newMatchDialog_->changeBaseDir();
      return Event_Processed;
#endif
    }
  }

  if (type == wxEVT_KEY_UP) {
    newMatchDialog_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    newMatchDialog_->setMnemonicLabels(false);
  }

  // TODO: Do we need to handle tab navigation manually on Windows? Nothing
  //       working at all on Windows 8 for me right now.
  return Event_Skip;
}
