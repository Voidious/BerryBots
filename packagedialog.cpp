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
#include "packagedialog.h"
#include "bbwx.h"

PackageDialog::PackageDialog(const char *title, PackageDialogListener *listener,
    MenuBarMaker *menuBarMaker) : wxFrame(NULL, wxID_ANY, title,
        wxPoint(50, 50), wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;
  numItems_ = 0;
  
  mainPanel_ = new wxPanel(this);
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  selectListBox_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                                 wxSize(275, 225), 0, NULL, wxLB_SORT);
  topSizer->Add(selectListBox_);
  topSizer->AddSpacer(5);

  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  versionLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "Version:");
  versionText_ = new wxTextCtrl(mainPanel_, wxID_ANY, "1.0", wxDefaultPosition,
                                wxSize(70, 23));
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
  wxBoxSizer *versionSizer = new wxBoxSizer(wxHORIZONTAL);
  versionSizer->Add(versionLabel_, 0, wxALIGN_CENTER);
  versionSizer->AddSpacer(5);
  versionSizer->Add(versionText_, 0, wxALIGN_CENTER);
  rightSizer->Add(versionSizer);
  packageLabel_.Append("    &");
  packageLabel_.Append(title);
  packageLabel_.Append("!    ");
  modifiedPackageLabel_.Append("&");
  modifiedPackageLabel_.Append(title);
#ifdef __WXOSX__
  modifiedPackageLabel_.Append("! \u2318P");
#else
  modifiedPackageLabel_.Append("!  alt-P");
#endif
  packageButton_ = new wxButton(mainPanel_, wxID_ANY, packageLabel_,
      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  rightSizer->AddSpacer(5);
  rightSizer->Add(packageButton_, 0, wxEXPAND | wxALIGN_BOTTOM);
  topSizer->Add(rightSizer, 0, wxEXPAND);

  borderSizer_ = new wxBoxSizer(wxHORIZONTAL);
  borderSizer_->Add(topSizer, 0, wxALL, 12);
  mainPanel_->SetSizerAndFit(borderSizer_);
  SetSizerAndFit(mainSizer_);

  versionText_->MoveAfterInTabOrder(selectListBox_);
  packageButton_->MoveAfterInTabOrder(versionText_);
  refreshButton_->MoveAfterInTabOrder(packageButton_);
  selectListBox_->MoveAfterInTabOrder(refreshButton_);

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(PackageDialog::onActivate));
  Connect(this->GetId(), wxEVT_SHOW, wxShowEventHandler(PackageDialog::onShow));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(PackageDialog::onClose));
  Connect(packageButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(PackageDialog::onPackage));
  Connect(wxID_REFRESH, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(PackageDialog::onRefreshFiles));

  eventFilter_ = new PackageEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

PackageDialog::~PackageDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete selectListBox_;
  delete versionLabel_;
  delete versionText_;
  delete refreshButton_;
  delete keyboardLabel_;
  delete mainPanel_;
}

void PackageDialog::clearItems() {
  selectListBox_->Clear();
  numItems_ = 0;
}

void PackageDialog::addItem(char *name) {
  selectListBox_->Append(wxString(name));
  numItems_++;
  if (selectListBox_->GetCount() > 0) {
    selectListBox_->SetFirstItem(0);
  }
}

void PackageDialog::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
  }
  SetSizerAndFit(mainSizer_);
}

void PackageDialog::onShow(wxShowEvent &event) {
  initialFocus();
}

void PackageDialog::onClose(wxCommandEvent &event) {
  listener_->cancel();
}

void PackageDialog::onPackage(wxCommandEvent &event) {
  packageSelectedItem();
}

void PackageDialog::packageSelectedItem() {
  wxArrayInt selectedIndex;
  selectListBox_->GetSelections(selectedIndex);
  wxString versionString = versionText_->GetValue();
  if (selectedIndex.Count() != 0 && versionString.length() > 0) {
    wxString selectedItem = selectListBox_->GetString(*(selectedIndex.begin()));
    char *name = new char[selectedItem.length() + 1];

#ifdef __WINDOWS__
    strcpy(name, selectedItem.c_str());
#else
    strcpy(name, selectedItem.fn_str());
#endif

    char *version = new char[versionString.length() + 1];
#ifdef __WINDOWS__
    strcpy(version, versionString.c_str());
#else
    strcpy(version, versionString.fn_str());
 #endif

    listener_->package(name, version, false);
  }
}

void PackageDialog::onRefreshFiles(wxCommandEvent &event) {
  refreshFiles();
}

void PackageDialog::refreshFiles() {
  listener_->refreshFiles();
}

void PackageDialog::onEscape() {
  listener_->cancel();
}

void PackageDialog::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    refreshButton_->SetLabel("&Refresh \u2318R");
#else
    refreshButton_->SetLabel("&Refresh  alt-R");
#endif
    packageButton_->SetLabel(modifiedPackageLabel_);
  } else {
    refreshButton_->SetLabel("    &Refresh    ");
    packageButton_->SetLabel(packageLabel_);
  }
}

void PackageDialog::initialFocus() {
  selectListBox_->SetFocus();
}

PackageEventFilter::PackageEventFilter(PackageDialog *dialog) {
  packageDialog_ = dialog;
}

PackageEventFilter::~PackageEventFilter() {
  
}

int PackageEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && packageDialog_->IsActive()) {
    packageDialog_->setMnemonicLabels(modifierDown);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      packageDialog_->onEscape();
      return Event_Processed;
#ifdef __WXOSX__
    // Mac OS X doesn't handle mnemonics, so add some manual keyboard shortcuts.
    } else if (keyEvent->GetUnicodeKey() == 'P' && modifierDown) {
      packageDialog_->packageSelectedItem();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'R' && modifierDown) {
      packageDialog_->refreshFiles();
      return Event_Processed;
#endif
    }
  }

  if (type == wxEVT_KEY_UP) {
    packageDialog_->setMnemonicLabels(modifierDown);
  }
  // TODO: Do we need to handle tab navigation manually on Windows? Nothing
  //       working at all on Windows 8 for me right now.
  return Event_Skip;
}
