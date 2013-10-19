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

#include <wx/wx.h>
#include "runnerdialog.h"
#include "bbwx.h"

RunnerDialog::RunnerDialog(RunnerDialogListener *listener,
                           MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, "Game Runner", wxPoint(50, 50), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;
  numItems_ = 0;
  
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
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *ctrlSizer = new wxBoxSizer(wxHORIZONTAL);
  itemSelect_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                              wxSize(275, 225), 0, NULL, wxLB_SORT);
  ctrlSizer->Add(itemSelect_);
  ctrlSizer->AddSpacer(5);
  
  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  refreshButton_ = new wxButton(mainPanel_, wxID_REFRESH, "    &Refresh    ");
  rightSizer->Add(refreshButton_, 0, wxEXPAND | wxALIGN_CENTER);
  rightSizer->AddStretchSpacer(1);
#ifdef __WXOSX__
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "\u2318 hotkeys");
#else
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "ALT hotkeys");
#endif
  rightSizer->Add(keyboardLabel_, 0, wxALIGN_CENTER);
  rightSizer->AddStretchSpacer(1);
  launchLabel_.Append("    ");
  launchLabel_.Append("&Launch");
  launchLabel_.Append("    ");
  modifiedLaunchLabel_.Append("&Launch");
#ifdef __WXOSX__
  modifiedLaunchLabel_.Append(" \u2318L");
#else
  modifiedLaunchLabel_.Append("  alt-L");
#endif
  launchButton_ = new wxButton(mainPanel_, wxID_ANY, launchLabel_,
                               wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  rightSizer->AddSpacer(5);
  rightSizer->Add(launchButton_, 0, wxEXPAND | wxALIGN_BOTTOM);
  ctrlSizer->Add(rightSizer, 0, wxEXPAND);

  topSizer->Add(ctrlSizer);
  topSizer->AddSpacer(10);
  wxString warning;
  wxStaticText *warningText1 = new wxStaticText(mainPanel_, wxID_ANY,
      "WARNING: Game Runners can access your disk!");
  wxStaticText *warningText2 = new wxStaticText(mainPanel_, wxID_ANY,
      "Only run programs you trust or wrote yourself.");
  topSizer->Add(warningText1, 0, wxALIGN_CENTER);
  topSizer->Add(warningText2, 0, wxALIGN_CENTER);

  borderSizer_ = new wxBoxSizer(wxHORIZONTAL);
  borderSizer_->Add(topSizer, 0, wxALL, 12);
  mainPanel_->SetSizerAndFit(borderSizer_);
  SetSizerAndFit(mainSizer_);
  
  launchButton_->MoveAfterInTabOrder(itemSelect_);
  refreshButton_->MoveAfterInTabOrder(launchButton_);
  itemSelect_->MoveAfterInTabOrder(refreshButton_);
  
  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(RunnerDialog::onActivate));
  Connect(this->GetId(), wxEVT_SHOW, wxShowEventHandler(RunnerDialog::onShow));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(RunnerDialog::onClose));
  Connect(launchButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(RunnerDialog::onLaunch));
  Connect(wxID_REFRESH, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(RunnerDialog::onRefreshFiles));
  Connect(itemSelect_->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerDialog::onSelectItem));
#ifdef __WINDOWS
  // On Windows XP, not redrawing the game while interrupted (dialog shown)
  // creates visual artifacts, so manually redraw it on update UI in Windows.
  // The same kills the framerate in Linux/GTK. Either way works on Mac/Cocoa.
  Connect(this->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerDialog::onUpdateUi));
#endif
  
  eventFilter_ = new RunnerEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

RunnerDialog::~RunnerDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
}

void RunnerDialog::clearItems() {
  itemSelect_->Clear();
  numItems_ = 0;
}

void RunnerDialog::addItem(char *name) {
  itemSelect_->Append(wxString(name));
  numItems_++;
  if (itemSelect_->GetCount() > 0) {
    itemSelect_->SetFirstItem(0);
  }
}

void RunnerDialog::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
    Fit();
  }
}

void RunnerDialog::onShow(wxShowEvent &event) {
  if (event.IsShown()) {
    focusItemSelect();
  }
}

void RunnerDialog::onClose(wxCommandEvent &event) {
  listener_->onClose();
}

void RunnerDialog::onLaunch(wxCommandEvent &event) {
  launchSelectedItem();
}

void RunnerDialog::launchSelectedItem() {
  wxArrayInt selectedIndex;
  itemSelect_->GetSelections(selectedIndex);
  if (selectedIndex.Count() != 0) {
    wxString selectedItem = itemSelect_->GetString(*(selectedIndex.begin()));
    char *name = new char[selectedItem.length() + 1];
    
#ifdef __WINDOWS__
    strcpy(name, selectedItem.c_str());
#else
    strcpy(name, selectedItem.fn_str());
#endif
    
    listener_->launch(name);
    delete name;
  }
}

void RunnerDialog::onRefreshFiles(wxCommandEvent &event) {
  refreshFiles();
}

void RunnerDialog::refreshFiles() {
  listener_->refreshFiles();
}

void RunnerDialog::onEscape() {
  listener_->onEscape();
}

void RunnerDialog::onSelectItem(wxUpdateUIEvent &event) {
  validateButtons();
}

void RunnerDialog::onUpdateUi(wxUpdateUIEvent &event) {
  listener_->onUpdateUi();
}

void RunnerDialog::validateButtons() {
  validateButtonSelectedListBox(launchButton_, itemSelect_);
}

void RunnerDialog::validateButtonSelectedListBox(wxButton *button,
                                                 wxListBox *listBox) {
  wxArrayInt selectedIndex;
  listBox->GetSelections(selectedIndex);
  if (selectedIndex.Count() > 0) {
    button->Enable();
  } else {
    button->Disable();
  }
}

void RunnerDialog::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    refreshButton_->SetLabel("&Refresh \u2318R");
#else
    refreshButton_->SetLabel("&Refresh  alt-R");
#endif
    launchButton_->SetLabel(modifiedLaunchLabel_);
  } else {
    refreshButton_->SetLabel("    &Refresh    ");
    launchButton_->SetLabel(launchLabel_);
  }
}

void RunnerDialog::focusItemSelect() {
  itemSelect_->SetFocus();
}

RunnerEventFilter::RunnerEventFilter(RunnerDialog *dialog) {
  runnerDialog_ = dialog;
}

RunnerEventFilter::~RunnerEventFilter() {
  
}

int RunnerEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif
  
  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && runnerDialog_->IsActive()) {
    runnerDialog_->setMnemonicLabels(modifierDown);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      runnerDialog_->onEscape();
      return Event_Processed;
    }
  }
  
  if (type == wxEVT_KEY_UP) {
    runnerDialog_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    runnerDialog_->setMnemonicLabels(false);
  }
  
  return Event_Skip;
}
