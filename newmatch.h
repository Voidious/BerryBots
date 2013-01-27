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

#ifndef BB_NEW_MATCH
#define BB_NEW_MATCH

#include <wx/wx.h>

class NewMatchListener {
  public:
    virtual void startMatch(const char *stagePath, char **teamPaths,
                            int numTeams) = 0;
    virtual void cancel() = 0;
};

class NewMatchDialog : public wxFrame {
  wxStaticText *stageLabel_;
  wxListBox *stageSelect_;
  wxStaticText *botsLabel_;
  wxListBox *botsSelect_;
  wxButton *addArrow_;
  wxButton *removeArrow_;
  wxButton *clearButton_;
  wxListBox *loadedBotsSelect_;
  wxButton *startButton_;
  unsigned int numStages_;
  unsigned int numBots_;
  unsigned int numLoadedBots_;
  NewMatchListener *listener_;

  public:
    NewMatchDialog();
    ~NewMatchDialog();
    void addStage(char *stage);
    void addBot(char *bot);
    void onAddBots(wxCommandEvent &event);
    void onRemoveBots(wxCommandEvent &event);
    void onClearBots(wxCommandEvent &event);
    void onStartMatch(wxCommandEvent &event);
    void onClose(wxCommandEvent &event);
    void onActivate(wxCommandEvent &event);
    void setListener(NewMatchListener *listener);
    void initMenus();
};

#endif
