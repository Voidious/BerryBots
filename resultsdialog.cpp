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
#include <wx/dataview.h>
#include "bbwx.h"
#include "resultsdialog.h"

ResultsDialog::ResultsDialog(Team **teams, int numTeams, wxPoint position)
    : wxFrame(NULL, wxID_ANY, "Results", position, wxDefaultSize,
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

  wxBoxSizer *tableSizer = new wxBoxSizer(wxHORIZONTAL);
  wxDataViewListCtrl *resultsCtrl =
  new wxDataViewListCtrl(this, wxID_ANY, wxPoint(0, 0),
                         wxSize(RESULTS_WIDTH, RESULTS_HEIGHT));
  resultsCtrl->AppendTextColumn("Rank", wxDATAVIEW_CELL_INERT, 50,
                                wxALIGN_CENTER);
  resultsCtrl->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 120,
                                wxALIGN_LEFT);

  bool hasScores = false;
  for (int x = 0; x < numTeams; x++) {
    if (teams[x]->result.score != 0) {
      hasScores = true;
      break;
    }
  }
  if (hasScores) {
    resultsCtrl->AppendTextColumn("Score", wxDATAVIEW_CELL_INERT, -1,
                                  wxALIGN_RIGHT);
  }

  TeamResult *firstResult = &(teams[0]->result);
  int numStats = firstResult->numStats;
  char **statKeys = 0;
  if (numStats > 0) {
    statKeys = new char*[firstResult->numStats];
    for (int x = 0; x < numStats; x++) {
      statKeys[x] = new char[strlen(firstResult->stats[x]->key) + 1];
      strcpy(statKeys[x], firstResult->stats[x]->key);
      resultsCtrl->AppendTextColumn(statKeys[x], wxDATAVIEW_CELL_INERT, -1,
                                    wxALIGN_RIGHT);
    }
  }

#ifdef __WXGTK__
  resultsCtrl->AppendTextColumn(" ", wxDATAVIEW_CELL_INERT, -1,
                                wxALIGN_RIGHT);
#endif

  for (int x = 0; x < numTeams; x++) {
    TeamResult *result = &(teams[x]->result);
    wxVector<wxVariant> data;
    if (result->rank == 0) {
      data.push_back("-");
    } else {
      data.push_back(wxString::Format(wxT("%i"), result->rank));
    }
    data.push_back(wxString(teams[x]->name));
    data.push_back(wxString::Format(wxT("%.2f"), result->score));
    for (int y = 0; y < numStats; y++) {
      char *key = statKeys[y];
      bool found = false;
      for (int z = 0; z < result->numStats; z++) {
        char *resultKey = result->stats[z]->key;
        if (strcmp(key, resultKey) == 0) {
          data.push_back(wxString::Format(wxT("%.2f"), result->stats[z]->value));
          found = true;
          break;
        }
      }
      if (!found) {
        data.push_back("-");
      }
    }
#ifdef __WXGTK__
    data.push_back(" ");
#endif
    resultsCtrl->AppendItem(data);
    data.clear();
  }

  tableSizer->Add(resultsCtrl, 0, wxEXPAND);
  SetSizerAndFit(tableSizer);
}

ResultsDialog::~ResultsDialog() {

}
