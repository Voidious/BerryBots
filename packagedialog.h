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

#ifndef PACKAGE_DIALOG_H
#define PACKAGE_DIALOG_H

#include <wx/wx.h>
#include "menubarmaker.h"

class PackageDialogListener {
  public:
    virtual void package(const char *name, const char *version,
                         bool obfuscate) = 0;
    virtual void refreshFiles() = 0;
    virtual void cancel() = 0;
    virtual ~PackageDialogListener() {};
};

class PackageDialog : public wxFrame {
  wxPanel *mainPanel_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  wxListBox *selectListBox_;
  wxStaticText *versionLabel_;
  wxTextCtrl *versionText_;
  wxButton *refreshButton_;
  wxButton *packageButton_;
  wxStaticText *keyboardLabel_;
  unsigned int numItems_;
  PackageDialogListener *listener_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  wxEventFilter *eventFilter_;
  wxString packageLabel_;
  wxString modifiedPackageLabel_;

  public:
    PackageDialog(const char *title, const char *buttonLabel,
                  PackageDialogListener *listener, MenuBarMaker *menuBarMaker);
    ~PackageDialog();
    void clearItems();
    void addItem(char *name);
    void onActivate(wxActivateEvent &event);
    void onShow(wxShowEvent &event);
    void onClose(wxCommandEvent &event);
    void onPackage(wxCommandEvent &event);
    void packageSelectedItem();
    void onRefreshFiles(wxCommandEvent &event);
    void refreshFiles();
    void onEscape();
    void setMnemonicLabels(bool modifierDown);
    void initialFocus();
};

class PackageEventFilter : public wxEventFilter {
  PackageDialog *packageDialog_;
  
  public:
    PackageEventFilter(PackageDialog *dialog);
    ~PackageEventFilter();
    virtual int FilterEvent(wxEvent& event);
};

#endif
