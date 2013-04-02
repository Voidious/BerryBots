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
#include "bbwx.h"
#include "gamerunner.h"
#include "runnerform.h"

RunnerForm::RunnerForm(const char *runnerName, RunnerFormElement **formElements,
                       int numElements, char **stageNames, int numStages,
                       char **shipNames, int numShips)
    : wxFrame(NULL, wxID_ANY, runnerName, wxPoint(50, 50), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE) {
  formElements_ = formElements;
  numElements_ = numElements;
  ok_ = done_ = false;

  mainPanel_ = new wxPanel(this);
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  cancelButton_ = new wxButton(mainPanel_, wxID_ANY, "Cance&l");
  okButton_ = new wxButton(mainPanel_, wxID_ANY, "&OK");

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *colSizer = new wxBoxSizer(wxVERTICAL);
  int colHeight = 0;
  int numCols = 0;
  for (int x = 0; x < numElements_; x++) {
    RunnerFormElement *element = formElements_[x];
    wxControl *control = addFormElement(colHeight, numCols, topSizer, colSizer,
        element->getName(), element->getType(), stageNames, numStages,
        shipNames, numShips);
    element->setControl(control);
  }
  addFormElement(colHeight, numCols, topSizer, colSizer, "",
      TYPE_OK_CANCEL, stageNames, numStages, shipNames, numShips);
  if (numCols++ > 0) {
    topSizer->AddSpacer(8);
  }
  topSizer->Add(colSizer);
  
  wxBoxSizer *borderSizer = new wxBoxSizer(wxHORIZONTAL);
  borderSizer->Add(topSizer, 0, wxALL, 12);
  mainPanel_->SetSizerAndFit(borderSizer);
  SetSizerAndFit(mainSizer_);

  Connect(cancelButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(RunnerForm::onCancel));
  Connect(okButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(RunnerForm::onOk));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(RunnerForm::onClose));

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
}

RunnerForm::~RunnerForm() {
  // TODO: figure out if I need to track and delete the controls
}

wxControl* RunnerForm::addFormElement(int &colHeight, int &numCols,
    wxBoxSizer *topSizer, wxBoxSizer *&colSizer, const char *name, int type,
    char **stageNames, int numStages, char **shipNames, int numShips) {
  if (colHeight > 0) {
    colSizer->AddSpacer(8);
  }
  int elementHeight = (type == TYPE_INTEGER_TEXT ? TEXT_HEIGHT
       : (type == TYPE_OK_CANCEL ? OK_CANCEL_HEIGHT : SELECT_HEIGHT));
  if (colHeight + elementHeight > MAX_COLUMN_HEIGHT) {
    if (numCols++ > 0) {
      topSizer->AddSpacer(8);
    }
    topSizer->Add(colSizer);
    colSizer = new wxBoxSizer(wxVERTICAL);
    colHeight = 0;
  }
  colHeight += elementHeight;

  std::string nameString(name);
  nameString.append(":");
  wxStaticText *nameLabel = new wxStaticText(mainPanel_, wxID_ANY, nameString);
  if (type == TYPE_INTEGER_TEXT) {
    wxTextCtrl *elementText = new wxTextCtrl(mainPanel_, wxID_ANY, "",
        wxDefaultPosition, wxSize(70, 23));
    wxBoxSizer *textSizer = new wxBoxSizer(wxHORIZONTAL);
    textSizer->Add(nameLabel, 0, wxALIGN_CENTER);
    textSizer->AddSpacer(5);
    textSizer->Add(elementText, 0, wxALIGN_CENTER);
    colSizer->Add(textSizer);
    return elementText;
  } else if (type == TYPE_OK_CANCEL) {
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer(1);
    buttonSizer->Add(cancelButton_);
    buttonSizer->AddSpacer(5);
    buttonSizer->Add(okButton_);
    buttonSizer->AddStretchSpacer(1);
    colSizer->AddSpacer(10);
    colSizer->Add(buttonSizer, 0, wxEXPAND | wxALIGN_CENTER);
    return 0;
  } else {
    wxListBox *itemSelect;
    if (type == TYPE_MULTI_SHIP_SELECT || type == TYPE_SINGLE_SHIP_SELECT) {
      long style = wxLB_SORT;
      if (type == TYPE_MULTI_SHIP_SELECT) {
        style |= wxLB_EXTENDED;
      }
      itemSelect = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                                 wxSize(275, 225), 0, NULL, style);
      for (int x = 0; x < numShips; x++) {
        itemSelect->Append(wxString(shipNames[x]));
      }
      itemSelect->SetFirstItem(0);
    } else { /* (type == TYPE_STAGE_SELECT) */
      itemSelect = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                                 wxSize(275, 225), 0, NULL, wxLB_SORT);
      for (int x = 0; x < numShips; x++) {
        itemSelect->Append(wxString(stageNames[x]));
      }
      itemSelect->SetFirstItem(0);
    }
    wxBoxSizer *selectSizer = new wxBoxSizer(wxVERTICAL);
    selectSizer->Add(nameLabel);
    selectSizer->AddSpacer(3);
    selectSizer->Add(itemSelect);
    colSizer->Add(selectSizer);
    return itemSelect;
  }
}

void RunnerForm::onCancel(wxCommandEvent &event) {
  done_ = true;
}

void RunnerForm::onOk(wxCommandEvent &event) {
  done_ = ok_ = true;
}

void RunnerForm::onClose(wxCommandEvent &event) {
  done_ = true;
  Hide();
}

bool RunnerForm::isDone() {
  return done_;
}

bool RunnerForm::isOk() {
  return ok_;
}
RunnerFormElement::RunnerFormElement(const char *name, int type,
                                     int maxStringValues) {
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  type_ = type;
  numStringValues_ = 0;
  maxStringValues_ = maxStringValues;
  stringValues_ = new char*[maxStringValues_];
  defaultStringValues_ = new char*[maxStringValues_];
  intValue_ = 0;
  defaultIntValue_ = 0;
  control_ = 0;
}

RunnerFormElement::~RunnerFormElement() {
  delete name_;
  for (int x = 0; x < numStringValues_; x++) {
    delete stringValues_[x];
  }
  delete stringValues_;
  for (int x = 0; x < numDefaultStringValues_; x++) {
    delete defaultStringValues_[x];
  }
  delete defaultStringValues_;
}

const char* RunnerFormElement::getName() {
  return name_;
}

int RunnerFormElement::getType() {
  return type_;
}

void RunnerFormElement::addStringValue(const char *value) {
  if (numStringValues_ < maxStringValues_) {
    char *newValue = new char[strlen(value) + 1];
    strcpy(newValue, value);
    stringValues_[numStringValues_++] = newValue;
  }
}

char** RunnerFormElement::getStringValues() {
  return stringValues_;
}

int RunnerFormElement::getNumStringValues() {
  return numStringValues_;
}

void RunnerFormElement::addDefaultStringValue(const char *value) {
  if (numDefaultStringValues_ < maxStringValues_) {
    char *newValue = new char[strlen(value) + 1];
    strcpy(newValue, value);
    defaultStringValues_[numDefaultStringValues_++] = newValue;
  }
}

char** RunnerFormElement::getDefaultStringValues() {
  return defaultStringValues_;
}

int RunnerFormElement::getNumDefaultStringValues() {
  return numDefaultStringValues_;
}

void RunnerFormElement::setIntegerValue(int value) {
  intValue_ = value;
}

int RunnerFormElement::getIntegerValue() {
  return intValue_;
}

void RunnerFormElement::setDefaultIntegerValue(int value) {
  defaultIntValue_ = value;
}

int RunnerFormElement::getDefaultIntegerValue() {
  return defaultIntValue_;
}

void RunnerFormElement::clearDefaults() {
  for (int x = 0; x < numDefaultStringValues_; x++) {
    delete defaultStringValues_[x];
  }
  numDefaultStringValues_ = 0;
  defaultIntValue_ = 0;
}

void RunnerFormElement::setControl(wxControl *control) {
  control_ = control;
}

wxControl* RunnerFormElement::getControl() {
  return control_;
}
