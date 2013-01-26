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
#include "packageship.h"
#include "bbwx.h"

PackageShipDialog::PackageShipDialog()
    : wxFrame(NULL, PACKAGE_SHIP_ID, "Package Ship...",
              wxPoint(50, 50), wxSize(380, 260),
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  botSelect_ = new wxListBox(this, -1, wxPoint(20, 20), wxSize(200, 200));
  versionLabel_ = new wxStaticText(this, -1, "Version:", wxPoint(230, 163));
  versionText_ = new wxTextCtrl(this, -1, "1.0", wxPoint(290, 160),
                                wxSize(70, 23));
  packageButton_ = new wxButton(this, PACKAGE_SHIP_BUTTON_ID, "Package!",
      wxPoint(224, 190), wxDefaultSize, wxBU_EXACTFIT);
  numBots_ = 0;
  listener_ = 0;
  
  Connect(PACKAGE_SHIP_ID, wxEVT_ACTIVATE,
          wxCommandEventHandler(PackageShipDialog::onActivate));
  Connect(PACKAGE_SHIP_ID, wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(PackageShipDialog::onClose));
  Connect(PACKAGE_SHIP_BUTTON_ID, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(PackageShipDialog::onPackage));
}

PackageShipDialog::~PackageShipDialog() {
  delete botSelect_;
  delete versionLabel_;
  delete versionText_;
}

void PackageShipDialog::addBot(char *bot) {
  botSelect_->Insert(wxString(bot), numBots_++);
  if (botSelect_->GetCount() > 0) {
    botSelect_->SetFirstItem(0);
  }
}

void PackageShipDialog::setListener(PackageShipDialogListener *listener) {
  listener_ = listener;
}

void PackageShipDialog::onActivate(wxCommandEvent &event) {
  botSelect_->SetFocus();
}

void PackageShipDialog::onClose(wxCommandEvent &event) {
  listener_->cancel();
}

void PackageShipDialog::onPackage(wxCommandEvent &event) {
  wxArrayInt selectedBotIndex;
  botSelect_->GetSelections(selectedBotIndex);
  wxString botVersion = versionText_->GetValue();
  if (selectedBotIndex.Count() != 0 && botVersion.length() > 0) {
    wxString selectedBot =
    botSelect_->GetString(*(selectedBotIndex.begin()));
    char *bot = new char[selectedBot.length() + 1];
    strcpy(bot, selectedBot.fn_str());
    
    char *version = new char[botVersion.length() + 1];
    strcpy(version, botVersion.fn_str());
    
    listener_->package(bot, version, false);
  }
}
