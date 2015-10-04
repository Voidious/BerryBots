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
#include "ResourcePath.hpp"
#include "outputconsole.h"

OutputConsole::OutputConsole(const char *title, int style,
                             MenuBarMaker *menuBarMaker)
    : wxFrame(NULL, wxID_ANY, title, wxPoint(50, 50), wxDefaultSize,
              wxDEFAULT_FRAME_STYLE) {
  menuBarMaker_ = menuBarMaker;
  outerSizer_ = new wxBoxSizer(wxVERTICAL);
  output_ = new wxTextCtrl(this, wxID_ANY, "", wxPoint(0, 0),
      wxSize(850, 550), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP,
      wxDefaultValidator);
  outerSizer_->Add(output_, 1, wxEXPAND);
  style_ = style;
  gfxCheckBox_ = 0;
  abortButton_ = 0;
  if (style != CONSOLE_PLAIN) {
    wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    wxPanel *bottomPanel = new wxPanel(this);
    bottomSizer->AddStretchSpacer(1);
    if (style == CONSOLE_SHIP_STAGE) {
      gfxCheckBox_ = new wxCheckBox(bottomPanel, wxID_ANY, "Enable Gfx");
      bottomSizer->Add(gfxCheckBox_, 0, wxALIGN_RIGHT | wxALL, 4);
    } else if (style == CONSOLE_RUNNER) {
      // On Windows/Mac, it's not picking up the cmd/alt up to hide the hotkey,
      // so just displaying it all the time for now.
#ifdef __WXOSX__
      abortButton_ = new wxButton(bottomPanel, wxID_ANY, "Abo&rt \u2318R");
#elif defined(__WINDOWS__)
      abortButton_ = new wxButton(bottomPanel, wxID_ANY, "Abo&rt  alt-R");
#else
      abortButton_ = new wxButton(bottomPanel, wxID_ANY, "    Abo&rt    ");
#endif
      bottomSizer->Add(abortButton_, 0, wxALIGN_RIGHT | wxALL, 4);
    }
    bottomPanel->SetSizerAndFit(bottomSizer);
    outerSizer_->Add(bottomPanel, 0, wxEXPAND);
  }
  listener_ = 0;
  menusInitialized_ = false;

#ifdef __WINDOWS__
  SetIcon(wxIcon(resourcePath() + BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));
#elif defined(__WXGTK__)
  SetIcon(wxIcon(resourcePath() + BBICON_128, wxBITMAP_TYPE_PNG));
#endif

#ifdef __WXOSX__
  defaultFontSize_ = 12;
#elif defined(__WINDOWS__)
  defaultFontSize_ = 9;
#else
  defaultFontSize_ = 10;
#endif

  fontSize_ = defaultFontSize_;
  output_->SetFont(wxFont(fontSize_, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                          wxFONTWEIGHT_NORMAL));
  SetSizerAndFit(outerSizer_);


  Connect(this->GetId(), wxEVT_ACTIVATE,
          wxActivateEventHandler(OutputConsole::onActivate));
  Connect(this->GetId(), wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(OutputConsole::onClose));
  if (style == CONSOLE_SHIP_STAGE) {
    Connect(gfxCheckBox_->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
            wxCommandEventHandler(OutputConsole::onCheck));
  } else if (style == CONSOLE_RUNNER) {
    Connect(abortButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(OutputConsole::onAbort));
  }
  
  eventFilter_ = new OutputConsoleEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

OutputConsole::~OutputConsole() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
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
  output_->SetInsertionPointEnd();
  output_->WriteText(text);
}

void OutputConsole::println(const char *text) {
  output_->SetInsertionPointEnd();
  output_->WriteText(text);
  output_->WriteText("\n");
}

void OutputConsole::println() {
  output_->SetInsertionPointEnd();
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

void OutputConsole::onCheck(wxCommandEvent &event) {
  if (listener_ != 0 && gfxCheckBox_ != 0) {
    listener_->onCheck(gfxCheckBox_->IsChecked());
  }
}

bool OutputConsole::isChecked() {
  return gfxCheckBox_->IsChecked();
}

void OutputConsole::onAbort(wxCommandEvent &event) {
  abort();
}

void OutputConsole::abort() {
  if (listener_ != 0 && abortButton_ != 0) {
    listener_->onAbort();
  }
}

// TODO: delta based on current text size, eg 36 => 42, not 38
void OutputConsole::increaseTextSize() {
  fontSize_ += 2;
  wxFont font = output_->GetFont();
  font.SetPointSize(fontSize_);
  output_->SetFont(font);
}

void OutputConsole::decreaseTextSize() {
  fontSize_ = std::max(4, fontSize_ -2);
  wxFont font = output_->GetFont();
  font.SetPointSize(fontSize_);
  output_->SetFont(font);
}

void OutputConsole::defaultTextSize() {
  fontSize_ = defaultFontSize_;
  wxFont font = output_->GetFont();
  font.SetPointSize(fontSize_);
  output_->SetFont(font);
}

int OutputConsole::getStyle() {
  return style_;
}

void OutputConsole::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
#ifdef __WXGTK__
  if (abortButton_ != 0) {
    if (modifierDown) {
      abortButton_->SetLabel("Abo&rt  alt-R");
    } else {
      abortButton_->SetLabel("    Abo&rt    ");
    }
  }
#endif
}

OutputConsoleEventFilter::OutputConsoleEventFilter(
    OutputConsole *outputConsole) {
  outputConsole_ = outputConsole;
}

OutputConsoleEventFilter::~OutputConsoleEventFilter() {
  
}

int OutputConsoleEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (outputConsole_->IsActive() && type == wxEVT_KEY_DOWN) {
    outputConsole_->setMnemonicLabels(modifierDown);
    wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
    int keyCode = keyEvent->GetKeyCode();
    if ((outputConsole_->getStyle() != CONSOLE_RUNNER && keyCode == WXK_ESCAPE)
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      outputConsole_->Close();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == '=' && keyEvent->ControlDown()) {
      outputConsole_->increaseTextSize();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == '-' && keyEvent->ControlDown()) {
      outputConsole_->decreaseTextSize();
      return Event_Processed;
    } else if (keyEvent->GetUnicodeKey() == '0' && keyEvent->ControlDown()) {
      outputConsole_->defaultTextSize();
      return Event_Processed;
    }
  }

  if (type == wxEVT_KEY_UP) {
    outputConsole_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    outputConsole_->setMnemonicLabels(false);
  }

  return Event_Skip;
}
