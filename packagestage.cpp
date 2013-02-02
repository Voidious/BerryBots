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
#include "packagestage.h"
#include "bbwx.h"

PackageStageDialog::PackageStageDialog(PackageStageDialogListener *listener)
    : wxFrame(NULL, PACKAGE_STAGE_ID, "Package Stage", wxPoint(50, 50),
              wxSize(380, 260),
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  stageSelect_ = new wxListBox(this, -1, wxPoint(20, 20), wxSize(200, 200));
  versionLabel_ = new wxStaticText(this, -1, "Version:", wxPoint(230, 163));
  versionText_ = new wxTextCtrl(this, -1, "1.0", wxPoint(290, 160),
                                wxSize(70, 23));
  packageButton_ = new wxButton(this, PACKAGE_STAGE_BUTTON_ID, "Package!",
      wxPoint(224, 190), wxDefaultSize, wxBU_EXACTFIT);
  numStages_ = 0;
  menusInitialized_ = false;

  Connect(PACKAGE_STAGE_ID, wxEVT_ACTIVATE,
          wxActivateEventHandler(PackageStageDialog::onActivate));
  Connect(PACKAGE_STAGE_ID, wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(PackageStageDialog::onClose));
  Connect(PACKAGE_STAGE_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(PackageStageDialog::onPackage));

  eventFilter_ = new PackageStageEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

PackageStageDialog::~PackageStageDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete stageSelect_;
  delete versionLabel_;
  delete versionText_;
}

void PackageStageDialog::clearStages() {
  stageSelect_->Clear();
  numStages_ = 0;
}

void PackageStageDialog::addStage(char *stage) {
  stageSelect_->Insert(wxString(stage), numStages_++);
  if (stageSelect_->GetCount() > 0) {
    stageSelect_->SetFirstItem(0);
  }
}

void PackageStageDialog::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(listener_->getNewMenuBar());
    menusInitialized_ = true;
  }
  stageSelect_->SetFocus();
}

void PackageStageDialog::onClose(wxCommandEvent &event) {
  listener_->cancel();
}

void PackageStageDialog::onPackage(wxCommandEvent &event) {
  packageSelectedStage();
}

void PackageStageDialog::packageSelectedStage() {
  wxArrayInt selectedStageIndex;
  stageSelect_->GetSelections(selectedStageIndex);
  wxString stageVersion = versionText_->GetValue();
  if (selectedStageIndex.Count() != 0 && stageVersion.length() > 0) {
    wxString selectedStage =
        stageSelect_->GetString(*(selectedStageIndex.begin()));
    char *stage = new char[selectedStage.length() + 1];
    strcpy(stage, selectedStage.fn_str());

    char *version = new char[stageVersion.length() + 1];
    strcpy(version, stageVersion.fn_str());

    listener_->package(stage, version, false);
  }
}

void PackageStageDialog::onEscape() {
  listener_->cancel();
}

PackageStageEventFilter::PackageStageEventFilter(PackageStageDialog *dialog) {
  packageStageDialog_ = dialog;
}

PackageStageEventFilter::~PackageStageEventFilter() {
  
}

int PackageStageEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && packageStageDialog_->IsActive()) {
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE) {
      packageStageDialog_->onEscape();
    } else if (keyEvent->GetUnicodeKey() == 'P' && keyEvent->ControlDown()) {
      packageStageDialog_->packageSelectedStage();
    }
#ifdef __WXOSX__
    if (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown()) {
      packageStageDialog_->onEscape();
    }
#endif
  }
  return Event_Skip;
}
