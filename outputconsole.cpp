/*
  Copyright (C) 2012-2013 - Voidious

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
#include "bbwx.h"
#include "outputconsole.h"

OutputConsole::OutputConsole(const char *title, bool enableCheckBox,
                             MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, title, wxPoint(50, 50), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE) {
  enableCheckBox_ = enableCheckBox;
  menuBarMaker_ = menuBarMaker;
  outerSizer_ = new wxBoxSizer(wxVERTICAL);
  output_ = new wxTextCtrl(this, wxID_ANY, "", wxPoint(0, 0),
      wxSize(650, 450), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP,
      wxDefaultValidator);
  outerSizer_->Add(output_, 1, wxEXPAND);
  gfxCheckBox_ = 0;
  if (enableCheckBox_) {
    wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    wxPanel *bottomPanel = new wxPanel(this);
    gfxCheckBox_ = new wxCheckBox(bottomPanel, wxID_ANY, "Enable Gfx");
    bottomSizer->AddStretchSpacer(1);
    bottomSizer->Add(gfxCheckBox_, 0, wxALIGN_RIGHT | wxALL, 4);
    bottomPanel->SetSizerAndFit(bottomSizer);
    outerSizer_->Add(bottomPanel, 0, wxEXPAND);
  }
  listener_ = 0;

#ifdef __WINDOWS__
  SetIcon(wxIcon(BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));
#elif defined(__WXGTK__)
  SetIcon(wxIcon(BBICON_128, wxBITMAP_TYPE_PNG));
#endif

#ifdef __WXOSX__
  defaultFontSize_ = 12;
#elif defined(__WINDOWS__)
  defaultFontSize_ = 9;
#else
  defaultFontSize_ = 10;
#endif

  fontSize_ = defaultFontSize_;
  SetSizerAndFit(outerSizer_);

  menusInitialized_ = false;
  closeOnSpace_ = false;

  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(OutputConsole::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(OutputConsole::onClose));
  if (enableCheckBox_) {
    Connect(gfxCheckBox_->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
            wxCommandEventHandler(OutputConsole::onCheck));
  }
  
  eventFilter_ = new OutputConsoleEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

OutputConsole::~OutputConsole() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
  delete output_;
  if (gfxCheckBox_ != 0) {
    delete gfxCheckBox_;
  }
  if (listener_ != 0) {
    delete listener_;
  }
}

void OutputConsole::setListener(ConsoleListener *listener) {
  if (listener_ != 0) {
    delete listener_;
  }
  listener_ = listener;
}

void OutputConsole::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
    outerSizer_->Layout();
    Fit();
  }
  if (event.GetActive() && listener_ != 0) {
    listener_->onActive();
  }
  if (gfxCheckBox_ != 0) {
    gfxCheckBox_->SetFocus();
  }
}

// This is the only wxWidgets dialog that needs to be drawn concurrently with
// the main SFML window. We get a lot of flickering in the main window if we
// don't override this.
void OutputConsole::OnEraseBackground(wxEraseEvent&) {

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
  if (listener_ != 0) {
    listener_->onClose();
  }
}

void OutputConsole::onSpace() {
  if (closeOnSpace_) {
    Close();
  }
}

void OutputConsole::onCheck(wxCommandEvent &event) {
  if (listener_ != 0 && gfxCheckBox_ != 0) {
    listener_->onCheck(gfxCheckBox_->IsChecked());
  }
}

bool OutputConsole::isChecked() {
  return gfxCheckBox_->IsChecked();
}

// TODO: delta based on current text size, eg 36 => 42, not 38
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

void OutputConsole::setCloseOnSpace() {
  closeOnSpace_ = true;
}

OutputConsoleEventFilter::OutputConsoleEventFilter(
    OutputConsole *outputConsole) {
  outputConsole_ = outputConsole;
}

OutputConsoleEventFilter::~OutputConsoleEventFilter() {
  
}

int OutputConsoleEventFilter::FilterEvent(wxEvent& event) {
  const wxEventType type = event.GetEventType();
  if (outputConsole_->IsActive() && type == wxEVT_KEY_DOWN) {
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      outputConsole_->Close();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == '=' && keyEvent->ControlDown()) {
      outputConsole_->increaseTextSize();
    } else if (keyEvent->GetUnicodeKey() == '-' && keyEvent->ControlDown()) {
      outputConsole_->decreaseTextSize();
    } else if (keyEvent->GetUnicodeKey() == '0' && keyEvent->ControlDown()) {
      outputConsole_->defaultTextSize();
    } else if (keyEvent->GetUnicodeKey() == ' ') {
      outputConsole_->onSpace();
    }
  }
  return Event_Skip;
}
