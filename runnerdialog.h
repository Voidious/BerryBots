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

#ifndef RUNNER_DIALOG_H
#define RUNNER_DIALOG_H

#include <wx/wx.h>
#include "menubarmaker.h"

class RunnerDialogListener {
  public:
    virtual void launch(const char *name) = 0;
    virtual void refreshFiles() = 0;
    virtual void onClose() = 0;
    virtual void onEscape() = 0;
    virtual void onUpdateUi() = 0;
    virtual ~RunnerDialogListener() {};
};

class RunnerDialog : public wxFrame {
  wxPanel *mainPanel_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  wxListBox *itemSelect_;
  wxButton *refreshButton_;
  wxButton *launchButton_;
  wxStaticText *keyboardLabel_;
  int numItems_;
  RunnerDialogListener *listener_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  wxEventFilter *eventFilter_;
  wxString launchLabel_;
  wxString modifiedLaunchLabel_;
  
public:
  RunnerDialog(RunnerDialogListener *listener, MenuBarMaker *menuBarMaker);
  ~RunnerDialog();
  void clearItems();
  void addItem(char *name);
  void onActivate(wxActivateEvent &event);
  void onShow(wxShowEvent &event);
  void onClose(wxCommandEvent &event);
  void onLaunch(wxCommandEvent &event);
  void launchSelectedItem();
  void onRefreshFiles(wxCommandEvent &event);
  void refreshFiles();
  void onEscape();
  void onSelectItem(wxUpdateUIEvent &event);
  void onUpdateUi(wxUpdateUIEvent &event);
  void validateButtons();
  void validateButtonSelectedListBox(wxButton *button, wxListBox *listBox);
  void setMnemonicLabels(bool modifierDown);
  void focusItemSelect();
};

class RunnerEventFilter : public wxEventFilter {
  RunnerDialog *runnerDialog_;
  
public:
  RunnerEventFilter(RunnerDialog *dialog);
  ~RunnerEventFilter();
  virtual int FilterEvent(wxEvent& event);
};

#endif
