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

PackageDialog::PackageDialog(const char *title, PackageDialogListener *listener)
    : wxFrame(NULL, wxID_ANY, title, wxPoint(50, 50), wxSize(400, 260),
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  selectListBox_ = new wxListBox(this, -1, wxPoint(20, 20), wxSize(200, 200), 0,
                                 NULL, wxLB_SORT);
  includeSrcCheckBox_ = new wxCheckBox(
      this, wxID_ANY, "Include source code", wxPoint(230, 168));
  includeSrcCheckBox_->SetValue(true);
  versionLabel_ = new wxStaticText(this, -1, "Version:", wxPoint(230, 140));
  versionText_ = new wxTextCtrl(this, -1, "1.0", wxPoint(290, 137),
                                wxSize(70, 23));
  packageButton_ = new wxButton(this, wxID_ANY, "Package!",
      wxPoint(224, 190), wxDefaultSize, wxBU_EXACTFIT);
  numItems_ = 0;
  menusInitialized_ = false;

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(PackageDialog::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(PackageDialog::onClose));
  Connect(packageButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(PackageDialog::onPackage));

  eventFilter_ = new PackageEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

PackageDialog::~PackageDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete selectListBox_;
  delete includeSrcCheckBox_;
  delete versionLabel_;
  delete versionText_;
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
    this->SetMenuBar(listener_->getNewMenuBar());
    menusInitialized_ = true;
  }
  selectListBox_->SetFocus();
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
    strcpy(name, selectedItem.fn_str());
    char *version = new char[versionString.length() + 1];
    strcpy(version, versionString.fn_str());

    listener_->package(name, version, !includeSrcCheckBox_->IsChecked());
  }
}

void PackageDialog::onEscape() {
  listener_->cancel();
}

PackageEventFilter::PackageEventFilter(PackageDialog *dialog) {
  packageDialog_ = dialog;
}

PackageEventFilter::~PackageEventFilter() {
  
}

int PackageEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && packageDialog_->IsActive()) {
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE) {
      packageDialog_->onEscape();
    } else if (keyEvent->GetUnicodeKey() == 'P' && keyEvent->ControlDown()) {
      packageDialog_->packageSelectedItem();
    }
    if (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown()) {
      packageDialog_->onEscape();
    }
  }
  return Event_Skip;
}
