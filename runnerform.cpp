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
#include <wx/valnum.h>
#include "bbwx.h"
#include "ResourcePath.hpp"
#include "gamerunner.h"
#include "runnerform.h"

RunnerForm::RunnerForm(const char *runnerName, RunnerFormElement **formElements,
    int numElements, char **stageNames, int numStages, char **shipNames,
    int numShips, const char *message, RunnerFormListener *listener)
    : wxFrame(NULL, wxID_ANY, runnerName, wxPoint(50, 50), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  formElements_ = formElements;
  numElements_ = numElements;
  ok_ = done_ = false;
  listener_ = listener;
  if (message == 0) {
    message_ = 0;
  } else {
    message_ = new char[strlen(message) + 1];
    strcpy(message_, message);
  }

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

  mainPanel_ = new wxPanel(this);
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  cancelButton_ = new wxButton(mainPanel_, wxID_ANY, "    Cance&l    ");
  okButton_ = new wxButton(mainPanel_, wxID_ANY, "    &OK    ");

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *colSizer = new wxBoxSizer(wxVERTICAL);
  int colHeight = 0;
  int numCols = 0;
  for (int x = 0; x < numElements_; x++) {
    RunnerFormElement *element = formElements_[x];
    wxControl *control = addFormElement(colHeight, numCols, topSizer, colSizer,
        element->getName(), element->getType(), stageNames, numStages,
        shipNames, numShips);
    setFormValues(control, element);
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
  // On Windows XP, not redrawing the game while interrupted (dialog shown)
  // creates visual artifacts, so manually redraw it on update UI in Windows.
  // The same kills the framerate in Linux/GTK. Either way works on Mac/Cocoa.
  Connect(this->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerForm::onUpdateUi));
#endif

  eventFilter_ = new RunnerFormEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

RunnerForm::~RunnerForm() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  if (message_ != 0) {
    delete message_;
  }
}

wxControl* RunnerForm::addFormElement(int &colHeight, int &numCols,
    wxBoxSizer *topSizer, wxBoxSizer *&colSizer, const char *name, int type,
    char **stageNames, int numStages, char **shipNames, int numShips) {
  if (colHeight > 0) {
    colSizer->AddSpacer(8);
  }
  int elementHeight = getHeight(type);
  if (colHeight + elementHeight > MAX_COLUMN_HEIGHT) {
    if (numCols++ > 0) {
      topSizer->AddSpacer(8);
    }
    topSizer->Add(colSizer);
    colSizer = new wxBoxSizer(wxVERTICAL);
    colHeight = 0;
  }
  colHeight += elementHeight;

  switch (type) {
    case TYPE_STAGE_SELECT:
    case TYPE_SINGLE_SHIP_SELECT:
    case TYPE_MULTI_SHIP_SELECT:
      return addSelectElement(name, type, stageNames, numStages, shipNames,
                              numShips, colSizer);
    case TYPE_INTEGER_TEXT:
      return addTextElement(name, colSizer);
    case TYPE_CHECKBOX:
      return addCheckboxElement(name, colSizer);
    case TYPE_OK_CANCEL:
      addOkCancelElement(colSizer);
      return 0;
    default:
      return 0;
  }
}

wxControl* RunnerForm::addSelectElement(const char *name, int type,
    char **stageNames, int numStages, char **shipNames, int numShips,
    wxSizer *colSizer) {
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
    for (int x = 0; x < numStages; x++) {
      itemSelect->Append(wxString(stageNames[x]));
    }
    itemSelect->SetFirstItem(0);
  }
  wxBoxSizer *selectSizer = new wxBoxSizer(wxVERTICAL);
  selectSizer->Add(getNameLabel(name));
  selectSizer->AddSpacer(3);
  selectSizer->Add(itemSelect);
  colSizer->Add(selectSizer);
  Connect(itemSelect->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerForm::onFormChange));
  return itemSelect;
}

wxControl* RunnerForm::addTextElement(const char *name, wxSizer *colSizer) {
  wxIntegerValidator<int> validator;
  wxTextCtrl *elementText = new wxTextCtrl(mainPanel_, wxID_ANY, "",
      wxDefaultPosition, wxSize(70, 23), 0, validator);
  wxBoxSizer *textSizer = new wxBoxSizer(wxHORIZONTAL);
  textSizer->Add(getNameLabel(name), 0, wxALIGN_CENTER);
  textSizer->AddSpacer(5);
  textSizer->Add(elementText, 0, wxALIGN_CENTER);
  colSizer->Add(textSizer);
  Connect(elementText->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerForm::onFormChange));
  return elementText;
}

wxControl* RunnerForm::addCheckboxElement(const char *name, wxSizer *colSizer) {
  wxCheckBox *checkbox = new wxCheckBox(mainPanel_, wxID_ANY, "");
  wxBoxSizer *textSizer = new wxBoxSizer(wxHORIZONTAL);
  textSizer->Add(getNameLabel(name), 0, wxALIGN_CENTER);
  textSizer->AddSpacer(5);
  textSizer->Add(checkbox, 0, wxALIGN_CENTER);
  colSizer->Add(textSizer);
  Connect(checkbox->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(RunnerForm::onFormChange));
  return checkbox;
}

void RunnerForm::addOkCancelElement(wxSizer *colSizer) {
  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->AddStretchSpacer(1);
  buttonSizer->Add(cancelButton_);
  buttonSizer->AddSpacer(5);
  buttonSizer->Add(okButton_);
  buttonSizer->AddStretchSpacer(1);
  colSizer->AddSpacer(10);
  colSizer->Add(buttonSizer, 0, wxEXPAND | wxALIGN_CENTER);
  if (message_ != 0) {
    wxStaticText *msgText = new wxStaticText(mainPanel_, wxID_ANY, message_);
    msgText->Wrap(265);
    colSizer->AddSpacer(15);
    colSizer->Add(msgText);
  }
}

wxStaticText* RunnerForm::getNameLabel(const char *name) {
  std::string nameString(name);
  nameString.append(":");
  return new wxStaticText(mainPanel_, wxID_ANY, nameString);
}

int RunnerForm::getHeight(int type) {
  switch (type) {
    case TYPE_STAGE_SELECT:
    case TYPE_SINGLE_SHIP_SELECT:
    case TYPE_MULTI_SHIP_SELECT:
      return SELECT_HEIGHT;
    case TYPE_INTEGER_TEXT:
      return TEXT_HEIGHT;
    case TYPE_CHECKBOX:
      return CHECKBOX_HEIGHT;
    default:
      return 0;
  }
}

void RunnerForm::setFormValues(wxControl *control,
                              RunnerFormElement *element) {
  if (element->getType() == TYPE_INTEGER_TEXT) {
    ((wxTextCtrl *) control)->SetValue(
        wxString::Format(wxT("%i"), element->getIntegerValue()));
  } else if (element->getType() == TYPE_CHECKBOX) {
    ((wxCheckBox *) control)->SetValue(element->getBooleanValue());
  } else {
    wxListBox *listBox = (wxListBox *) control;
    int numStringValues = element->getNumStringValues();
    char** stringValues = element->getStringValues();
    for (int x = 0; x < numStringValues; x++) {
      int numSelectStrings = listBox->GetCount();
      for (int y = 0; y < numSelectStrings; y++) {
        wxString itemName = listBox->GetString(y);
        if (strcmp(itemName.c_str(), stringValues[x]) == 0) {
          listBox->Select(y);
        }
      }
    }
  }
}

void RunnerForm::onCancel(wxCommandEvent &event) {
  cancel();
}

void RunnerForm::cancel() {
  done_ = true;
}

void RunnerForm::onOk(wxCommandEvent &event) {
  ok();
}

void RunnerForm::ok() {
  done_ = ok_ = true;
}

void RunnerForm::onClose(wxCommandEvent &event) {
  done_ = true;
  Hide();
}

void RunnerForm::onFormChange(wxUpdateUIEvent &event) {
  bool valid = true;
  for (int x = 0; x < numElements_; x++) {
    RunnerFormElement *element = formElements_[x];
    wxControl *control = element->getControl();
    int type = element->getType();
    if (type == TYPE_INTEGER_TEXT) {
      if (((wxTextCtrl *) control)->GetValue().size() == 0) {
        valid = false;
        break;
      }
    } else if (type == TYPE_SINGLE_SHIP_SELECT || type == TYPE_STAGE_SELECT) {
      wxListBox *listBox = (wxListBox *) control;
      wxArrayInt selectedIndex;
      listBox->GetSelections(selectedIndex);
      if (selectedIndex.Count() == 0) {
        valid = false;
        break;
      }
    }
  }
  if (valid) {
    okButton_->Enable();
  } else {
    okButton_->Disable();
  }
}

void RunnerForm::onUpdateUi(wxUpdateUIEvent &event) {
  listener_->onUpdateUi();
}

bool RunnerForm::isDone() {
  return done_;
}

bool RunnerForm::isOk() {
  return ok_;
}

void RunnerForm::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    cancelButton_->SetLabel("Cance&l \u2318L");
    okButton_->SetLabel("&OK \u2318O");
#else
    cancelButton_->SetLabel("Cance&l  alt-L");
    okButton_->SetLabel("&OK  alt-O");
#endif
  } else {
    cancelButton_->SetLabel("    Cance&l    ");
    okButton_->SetLabel("    &OK    ");
  }
}

RunnerFormElement::RunnerFormElement(const char *name, int type,
                                     int maxStringValues) {
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  type_ = type;
  numStringValues_ = 0;
  maxStringValues_ = maxStringValues;
  stringValues_ = new char*[maxStringValues_];
  intValue_ = 0;
  booleanValue_ = false;
  control_ = 0;
}

RunnerFormElement::~RunnerFormElement() {
  delete name_;
  for (int x = 0; x < numStringValues_; x++) {
    delete stringValues_[x];
  }
  delete stringValues_;
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

void RunnerFormElement::setIntegerValue(int value) {
  intValue_ = value;
}

int RunnerFormElement::getIntegerValue() {
  return intValue_;
}

void RunnerFormElement::setBooleanValue(bool value) {
  booleanValue_ = value;
}

int RunnerFormElement::getBooleanValue() {
  return booleanValue_;
}

void RunnerFormElement::clearValues() {
  for (int x = 0; x < numStringValues_; x++) {
    delete stringValues_[x];
  }
  numStringValues_ = 0;
  intValue_ = 0;
}

void RunnerFormElement::setControl(wxControl *control) {
  control_ = control;
}

wxControl* RunnerFormElement::getControl() {
  return control_;
}

RunnerFormEventFilter::RunnerFormEventFilter(RunnerForm *runnerForm) {
  runnerForm_ = runnerForm;
}

RunnerFormEventFilter::~RunnerFormEventFilter() {
  
}

int RunnerFormEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && runnerForm_->IsActive()) {
    runnerForm_->setMnemonicLabels(modifierDown);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      runnerForm_->cancel();
      return Event_Processed;
    }
  }

  if (type == wxEVT_KEY_UP) {
    runnerForm_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    runnerForm_->setMnemonicLabels(false);
  }

  return Event_Skip;
}
