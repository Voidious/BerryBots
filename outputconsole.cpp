/*
  Copyright (C) 2012 - Voidious

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

#include <algorithm>
#include <wx/wx.h>
#include "outputconsole.h"

OutputConsole::OutputConsole(const char *title, MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, title, wxPoint(50, 50), wxSize(600, 450),
              wxDEFAULT_FRAME_STYLE) {
  output_ = new wxTextCtrl(this, wxID_ANY, "", wxPoint(0, 0), wxSize(400, 350),
                          wxTE_MULTILINE | wxTE_READONLY, wxDefaultValidator);
#ifdef __WXOSX__
  defaultFontSize_ = 12;
#else
  defaultFontSize_ = 10;
#endif
  fontSize_ = defaultFontSize_;
  output_->SetFont(wxFont(fontSize_, wxFONTFAMILY_TELETYPE));

  menuBarMaker_ = menuBarMaker;
  menusInitialized_ = false;

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(OutputConsole::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(OutputConsole::onClose));
  eventFilter_ = new OutputConsoleEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

OutputConsole::~OutputConsole() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete output_;
}

void OutputConsole::onActivate(wxActivateEvent &event) {
#ifndef __WINDOWS__
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
  }
#endif
}

void OutputConsole::print(const char *text) {
  output_->WriteText(text);
}

void OutputConsole::println(const char *text) {
  output_->WriteText(text);
  output_->WriteText("\n");
}

void OutputConsole::println() {
  output_->WriteText("\n");
}

void OutputConsole::clear() {
  output_->Clear();
}

void OutputConsole::onClose(wxCommandEvent &event) {
  Hide();
}

// TODO: delta based on current text size, eg 36 => 42 vs 38
void OutputConsole::increaseTextSize() {
  fontSize_ += 2;
  output_->SetFont(wxFont(fontSize_, wxFONTFAMILY_TELETYPE));
}

void OutputConsole::decreaseTextSize() {
  fontSize_ = std::max(4, fontSize_ -2);
  output_->SetFont(wxFont(fontSize_, wxFONTFAMILY_TELETYPE));
}

void OutputConsole::defaultTextSize() {
  fontSize_ = defaultFontSize_;
  output_->SetFont(wxFont(fontSize_, wxFONTFAMILY_TELETYPE));
}

OutputConsoleEventFilter::OutputConsoleEventFilter(
    OutputConsole *outputConsole) {
  outputConsole_ = outputConsole;
}

OutputConsoleEventFilter::~OutputConsoleEventFilter() {
  
}

int OutputConsoleEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
  int keyCode = keyEvent->GetKeyCode();
  if (type == wxEVT_KEY_DOWN && outputConsole_->IsActive()) {
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      outputConsole_->Hide();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == '=' && keyEvent->ControlDown()) {
      outputConsole_->increaseTextSize();
    } else if (keyEvent->GetUnicodeKey() == '-' && keyEvent->ControlDown()) {
      outputConsole_->decreaseTextSize();
    } else if (keyEvent->GetUnicodeKey() == '0' && keyEvent->ControlDown()) {
      outputConsole_->defaultTextSize();
    }
  }
  return Event_Skip;
}
