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

#ifndef PACKAGE_STAGE_H
#define PACKAGE_STAGE_H

#include <wx/wx.h>

class PackageStageDialogListener {
  public:
    virtual wxMenuBar* getNewMenuBar() = 0;
    virtual void package(const char *stageName, const char *version,
                         bool nosrc) = 0;
    virtual void cancel() = 0;
    virtual ~PackageStageDialogListener() {};
};

// TODO: factor out common base class for this and PackageShipDialog

class PackageStageDialog : public wxFrame {
  wxListBox *stageSelect_;
  wxStaticText *versionLabel_;
  wxTextCtrl *versionText_;
  wxButton *packageButton_;
  unsigned int numStages_;
  PackageStageDialogListener *listener_;
  bool menusInitialized_;

  public:
    PackageStageDialog(PackageStageDialogListener *listener);
    ~PackageStageDialog();
    void addStage(char *stage);
    void onActivate(wxActivateEvent &event);
    void onClose(wxCommandEvent &event);
    void onPackage(wxCommandEvent &event);
};

#endif
