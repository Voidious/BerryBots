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

#include <string.h>
#include <wx/wx.h>
#include <wx/grid.h>
#include "bbwx.h"
#include "resultsdialog.h"

ResultsDialog::ResultsDialog(Team **teams, int numTeams, wxPoint center)
    : wxFrame(NULL, wxID_ANY, "Results", wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxMAXIMIZE_BOX)) {

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

  wxPanel *mainPanel = new wxPanel(this, wxID_ANY);
  wxBoxSizer *tableSizer = new wxBoxSizer(wxHORIZONTAL);
  wxGrid *resultsGrid =
      new wxGrid(mainPanel, wxID_ANY, wxPoint(0, 0), wxDefaultSize);

  bool hasScores = false;
  for (int x = 0; x < numTeams; x++) {
    if (teams[x]->result.score != 0) {
      hasScores = true;
      break;
    }
  }
  int baseCols = (hasScores ? 3 : 2);

  TeamResult *firstResult = &(teams[0]->result);
  int numStats = firstResult->numStats;
  char **statKeys = 0;
  if (numStats > 0) {
    statKeys = new char*[firstResult->numStats];
    for (int x = 0; x < numStats; x++) {
      statKeys[x] = new char[strlen(firstResult->stats[x]->key) + 1];
      strcpy(statKeys[x], firstResult->stats[x]->key);
      resultsGrid->SetColLabelValue(baseCols + x, statKeys[x]);
    }
  }

  int numResults = 0;
  for (int x = 0; x < numTeams; x++) {
    if (teams[x]->result.showResult) {
      numResults++;
    }
  }
  resultsGrid->CreateGrid(numResults, baseCols + numStats);
  resultsGrid->EnableEditing(false);
  resultsGrid->SetColumnWidth(0, 50);
  resultsGrid->SetColLabelValue(0, "Rank");
  resultsGrid->SetColumnWidth(1, 150);
  resultsGrid->SetColLabelValue(1, "Name");
  if (hasScores) {
    resultsGrid->SetColLabelValue(2, "Score");
  }
  for (int x = 0; x < numStats; x++) {
    resultsGrid->SetColLabelValue(baseCols + x, statKeys[x]);
  }
  resultsGrid->HideRowLabels();

  int resultIndex = 0;
  for (int x = 0; x < numTeams; x++) {
    TeamResult *result = &(teams[x]->result);
    if (result->showResult) {
      if (result->rank == 0) {
        resultsGrid->SetCellValue(resultIndex, 0, "-");
      } else {
        resultsGrid->SetCellValue(resultIndex, 0,
                                  wxString::Format(wxT("%i"), result->rank));
      }
      resultsGrid->SetCellAlignment(wxALIGN_CENTER, resultIndex, 0);
      resultsGrid->SetCellValue(resultIndex, 1,
                                wxString::Format(wxT(" %s"), teams[x]->name));
      if (hasScores) {
        resultsGrid->SetCellValue(resultIndex, 2,
                                  wxString::Format(wxT("%.2f "), result->score));
        resultsGrid->SetCellAlignment(wxALIGN_RIGHT, resultIndex, 2);
      }
      for (int y = 0; y < numStats; y++) {
        char *key = statKeys[y];
        bool found = false;
        for (int z = 0; z < result->numStats; z++) {
          char *resultKey = result->stats[z]->key;
          if (strcmp(key, resultKey) == 0) {
            resultsGrid->SetCellValue(resultIndex, baseCols + y,
                wxString::Format(wxT("%.2f "), result->stats[z]->value));
            resultsGrid->SetCellAlignment(wxALIGN_RIGHT, resultIndex,
                                          baseCols + y);
            found = true;
            break;
          }
        }
        if (!found) {
          resultsGrid->SetCellValue(resultIndex, baseCols + y, "-");
        }
      }
      resultIndex++;
    }
  }

  tableSizer->Add(resultsGrid, 0, wxEXPAND);
  wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(mainPanel);
  mainPanel->SetSizerAndFit(tableSizer);
  SetSizerAndFit(mainSizer);
  wxSize windowSize = GetSize();
  this->SetPosition(wxPoint(center.x - (windowSize.x / 2),
                            center.y - (windowSize.y / 2)));

  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(ResultsDialog::onClose));
  eventFilter_ = new ResultsEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

ResultsDialog::~ResultsDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
}

void ResultsDialog::onClose(wxCommandEvent &event) {
  Hide();
}

ResultsEventFilter::ResultsEventFilter(ResultsDialog *resultsDialog) {
  resultsDialog_ = resultsDialog;
}

ResultsEventFilter::~ResultsEventFilter() {
  
}

int ResultsEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  if (resultsDialog_->IsActive() && type == wxEVT_KEY_DOWN) {
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      resultsDialog_->Close();
      return Event_Processed;
    }
  }
  return Event_Skip;
}
