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

#ifndef OUTPUT_CONSOLE_H
#define OUTPUT_CONSOLE_H

#include <wx/wx.h>
#include "menubarmaker.h"

class ConsoleListener {
  public:
    virtual void onActive() = 0;
    virtual void onClose() = 0;
    virtual ~ConsoleListener() {};
};

class OutputConsole : public wxFrame {
  wxTextCtrl *output_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  wxEventFilter *eventFilter_;
  int defaultFontSize_;
  int fontSize_;
  bool closeOnSpace_;
  ConsoleListener *listener_;

  public:
    OutputConsole(const char *title, MenuBarMaker *menuBarMaker);
    ~OutputConsole();
    void setListener(ConsoleListener *listener);
    void onActivate(wxActivateEvent &event);
    void print(const char *text);
    void println(const char *text);
    void println();
    void clear();
    void onClose(wxCommandEvent &event);
    void onSpace();
    void increaseTextSize();
    void decreaseTextSize();
    void defaultTextSize();
    void setCloseOnSpace();
    
    virtual void OnEraseBackground(wxEraseEvent&);
};

class OutputConsoleEventFilter : public wxEventFilter {
  OutputConsole *outputConsole_;

  public:
    OutputConsoleEventFilter(OutputConsole *outputConsole);
    ~OutputConsoleEventFilter();
    virtual int FilterEvent(wxEvent& event);
};

#endif
