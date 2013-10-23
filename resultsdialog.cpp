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
#include <wx/datetime.h>
#include "bbwx.h"
#include "basedir.h"
#include "ResourcePath.hpp"
#include "filemanager.h"
#include "sysexec.h"
#include "resultsdialog.h"

ResultsDialog::ResultsDialog(const char *stageName, Team **teams, int numTeams,
    bool hasScores, wxPoint center, ReplayBuilder *replayBuilder)
    : wxFrame(NULL, wxID_ANY, "Results", wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxMAXIMIZE_BOX | wxRESIZE_BORDER)) {

  stageName_ = new char[strlen(stageName) + 1];
  replayFilename_ = 0;
  timestamp_ = 0;
  savedReplay_ = false;
  strcpy(stageName_, stageName);
  replayBuilder_ = replayBuilder;
  fileManager_ = new FileManager();
  systemExecutor_ = new SystemExecutor();

#ifdef __WINDOWS__
  SetIcon(wxIcon(resourcePath() + BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));
  
  // The 8-9 point default font size in Windows is much smaller than Mac/Linux.
  wxFont windowFont = GetFont();
  if (windowFont.GetPointSize() <= 9) {
    SetFont(windowFont.Larger());
  }
#elif __WXGTK__
  SetIcon(wxIcon(resourcePath() + BBICON_128, wxBITMAP_TYPE_PNG));
#endif

  mainPanel_ = new wxPanel(this, wxID_ANY);
  panelSizer_ = new wxBoxSizer(wxVERTICAL);
  wxGrid *resultsGrid =
      new wxGrid(mainPanel_, wxID_ANY, wxPoint(0, 0), wxDefaultSize);

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

  panelSizer_->Add(resultsGrid, 0, wxEXPAND);
  panelSizer_->AddSpacer(3);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  saveButton_ = new wxButton(mainPanel_, wxID_ANY, "    &Save Replay    ");
  viewButton_ = new wxButton(mainPanel_, wxID_ANY, "    &View Replay    ");
  buttonSizer->AddSpacer(3);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->Add(saveButton_, 0, wxEXPAND);
  buttonSizer->AddSpacer(5);
  buttonSizer->Add(viewButton_, 0, wxEXPAND);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->AddSpacer(3);

  panelSizer_->Add(buttonSizer, 0, wxEXPAND, 50);
  panelSizer_->AddSpacer(3);

  dialogSizer_ = new wxBoxSizer(wxHORIZONTAL);
  dialogSizer_->Add(mainPanel_);
  mainPanel_->SetSizerAndFit(panelSizer_);
  SetSizerAndFit(dialogSizer_);
  wxSize windowSize = GetSize();
  this->SetPosition(wxPoint(center.x - (windowSize.x / 2),
                            center.y - (windowSize.y / 2)));

  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(ResultsDialog::onClose));
  Connect(saveButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(ResultsDialog::onSaveReplay));
  Connect(viewButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(ResultsDialog::onViewReplay));

  eventFilter_ = new ResultsEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

ResultsDialog::~ResultsDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete saveButton_;
  delete viewButton_;
  delete stageName_;
  delete fileManager_;
  delete systemExecutor_;
  if (replayFilename_ != 0) {
    delete replayFilename_;
  }
  if (timestamp_ != 0) {
    delete timestamp_;
  }
  delete replayBuilder_; // inherited from the engine
}

void ResultsDialog::onClose(wxCommandEvent &event) {
  Hide();
}

void ResultsDialog::onSaveReplay(wxCommandEvent &event) {
  saveReplay();
}

void ResultsDialog::onViewReplay(wxCommandEvent &event) {
  viewReplay();
}

void ResultsDialog::saveReplay() {
  if (!savedReplay_) {
    replayFilename_ = generateFilename();
    wxDateTime dateTime;
    dateTime.SetToCurrent();
    if (timestamp_ != 0) {
      replayBuilder_->setTimestamp(timestamp_);
    }
    replayBuilder_->saveReplay(replayFilename_);
    displayFilename(replayFilename_);
    savedReplay_ = true;
    saveButton_->Disable();
  }
}

void ResultsDialog::viewReplay() {
  saveReplay();
  systemExecutor_->openHtmlFile(replayFilename_);
}

char* ResultsDialog::generateFilename() {
  char *absFilename = 0;
  do {
    if (absFilename != 0) {
      delete absFilename;
    }
    char *filename = newFilename();
    absFilename = fileManager_->getFilePath(getReplaysDir().c_str(), filename);
    delete filename;
    
  } while (fileManager_->fileExists(absFilename));

  return absFilename;
}

char* ResultsDialog::newFilename() {
  std::string filename(stageName_);
  filename.append("-");

  wxDateTime dateTime;
  dateTime.SetToCurrent();
  std::string timestamp(dateTime.Format("%Y.%m.%d-%H.%M.%S"));
  filename.append(timestamp);
  filename.append(".html");

  if (timestamp_ != 0) {
    delete timestamp_;
  }
  timestamp_ = new char[timestamp.length() + 1];
  strcpy(timestamp_, timestamp.c_str());

  char *newFilename = new char[filename.length() + 1];
  strcpy(newFilename, filename.c_str());
  return newFilename;
}

void ResultsDialog::displayFilename(const char *filename) {
  char *displayName;
  if (strlen(filename) > MAX_DISPLAY_NAME_LENGTH) {
    displayName = new char[MAX_DISPLAY_NAME_LENGTH + 1];
    int first = ((MAX_DISPLAY_NAME_LENGTH - 3) / 6);
    int last = MAX_DISPLAY_NAME_LENGTH - 3 - first;
    strncpy(displayName, filename, first);
    displayName[first] = displayName[first + 1] = displayName[first + 2] = '.';
    strncpy(
        &(displayName[first + 3]), &(filename[strlen(filename) - last]), last);
    displayName[MAX_DISPLAY_NAME_LENGTH] = '\0';
  } else {
    displayName = new char[strlen(filename) + 1];
    strcpy(displayName, filename);
  }
  wxBoxSizer *saveSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText *replayFilenameText = new wxStaticText(
      mainPanel_, wxID_ANY, wxString::Format("Saved: %s", displayName));
  saveSizer->AddSpacer(3);
  saveSizer->Add(replayFilenameText);
  saveSizer->AddSpacer(3);
  panelSizer_->Add(saveSizer, 0, wxALIGN_CENTER);
  panelSizer_->AddSpacer(3);
  mainPanel_->SetSizerAndFit(panelSizer_);
  SetSizerAndFit(dialogSizer_);
}

void ResultsDialog::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    saveButton_->SetLabel("&Save Replay \u2318S");
    viewButton_->SetLabel("&View Replay \u2318V");
#else
    saveButton_->SetLabel("&Save Replay  alt-S");
    viewButton_->SetLabel("&View Replay  alt-V");
#endif
  } else {
    saveButton_->SetLabel("    &Save Replay    ");
    viewButton_->SetLabel("    &View Replay    ");
  }
}

ResultsEventFilter::ResultsEventFilter(ResultsDialog *resultsDialog) {
  resultsDialog_ = resultsDialog;
}

ResultsEventFilter::~ResultsEventFilter() {
  
}

int ResultsEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (resultsDialog_->IsActive() && type == wxEVT_KEY_DOWN) {
    resultsDialog_->setMnemonicLabels(modifierDown);
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && modifierDown)) {
      resultsDialog_->Close();
      return Event_Processed;
#ifdef __WXOSX__
    // Mac OS X doesn't handle mnemonics, so add some manual keyboard shortcuts.
    } else if (keyEvent->GetUnicodeKey() == 'S' && modifierDown) {
      resultsDialog_->saveReplay();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == 'V' && modifierDown) {
      resultsDialog_->viewReplay();
      return Event_Processed;
#endif
    }
  }

  if (type == wxEVT_KEY_UP) {
    resultsDialog_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    resultsDialog_->setMnemonicLabels(false);
  }

  return Event_Skip;
}
