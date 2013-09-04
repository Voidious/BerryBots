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

#ifndef RESULTS_DIALOG_H
#define RESULTS_DIALOG_H

#include <wx/wx.h>
#include "bbutil.h"
#include "replaybuilder.h"

class ResultsDialog : public wxFrame {
  wxButton *saveButton_;
  wxButton *viewButton_;
  ReplayBuilder *replayBuilder_;
  wxEventFilter *eventFilter_;

  public:
    ResultsDialog(Team **teams, int numTeams, bool hasScores, wxPoint center,
                  ReplayBuilder *replayBuilder);
    ~ResultsDialog();
    void onClose(wxCommandEvent &event);
    void onSaveReplay(wxCommandEvent &event);
    void onViewReplay(wxCommandEvent &event);
    void saveReplay();
    void viewReplay();
    void setMnemonicLabels(bool modifierDown);
};

class ResultsEventFilter : public wxEventFilter {
  ResultsDialog *resultsDialog_;
  
  public:
    ResultsEventFilter(ResultsDialog *resultsDialog);
    ~ResultsEventFilter();
    virtual int FilterEvent(wxEvent& event);
};

#endif
