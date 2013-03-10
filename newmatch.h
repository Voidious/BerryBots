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
#include "menubarmaker.h"

class NewMatchListener {
  public:
    virtual void startMatch(const char *stagePath, char **teamPaths,
                            int numTeams) = 0;
    virtual void previewStage(const char *stagePath) = 0;
    virtual void refreshFiles() = 0;
    virtual void onClose() = 0;
    virtual void onEscape() = 0;
    virtual void onActive() = 0;
    virtual void onUpdateUi() = 0;
    virtual void reloadBaseDirs() = 0;
    virtual ~NewMatchListener() {};
};

class NewMatchDialog : public wxFrame {
  wxPanel *mainPanel_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  wxStaticText *stageLabel_;
  wxStaticText *previewLabel_;
  wxListBox *stageSelect_;
  wxStaticText *shipsLabel_;
  wxListBox *shipsSelect_;
  wxButton *addArrow_;
  wxButton *removeArrow_;
  wxButton *clearButton_;
  wxListBox *loadedShipsSelect_;
  wxButton *startButton_;
  wxButton *refreshButton_;
  wxButton *folderButton_;
  wxButton *browseStagesButton_;
  wxButton *browseShipsButton_;
  wxButton *browseApidocsButton_;
  wxStaticText *stagesBaseDirLabel_;
  wxStaticText *stagesBaseDirValueLabel_;
  wxStaticText *shipsBaseDirLabel_;
  wxStaticText *shipsBaseDirValueLabel_;
  wxStaticText *keyboardLabel_;
  int numStages_;
  int numShips_;
  int numLoadedShips_;
  NewMatchListener *listener_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  wxEventFilter *eventFilter_;

  public:
    NewMatchDialog(NewMatchListener *listener, MenuBarMaker *menuBarMaker);
    ~NewMatchDialog();
    void clearStages();
    void addStage(char *stage);
    void clearShips();
    void addShip(char *ship);
    void onActivate(wxActivateEvent &event);
    void onClose(wxCommandEvent &event);
    void onAddShips(wxCommandEvent &event);
    void addSelectedShips();
    void onRemoveShips(wxCommandEvent &event);
    void removeSelectedLoadedShips();
    void removeStaleLoadedShips();
    void onClearLoadedShips(wxCommandEvent &event);
    void clearLoadedShips();
    void onStartMatch(wxCommandEvent &event);
    void startMatch();
    void onRefreshFiles(wxCommandEvent &event);
    void refreshFiles();
    void onBrowseStages(wxCommandEvent &event);
    void onBrowseShips(wxCommandEvent &event);
    void onBrowseApidocs(wxCommandEvent &event);
    void onChangeBaseDir(wxCommandEvent &event);
    void onEscape();
    void onSelectStage(wxUpdateUIEvent &event);
    void onSelectShip(wxUpdateUIEvent &event);
    void onSelectLoadedShip(wxUpdateUIEvent &event);
    void onUpdateUi(wxUpdateUIEvent &event);
    void onSetBaseDirs();
    void previewSelectedStage();
    bool stageSelectHasFocus();
    bool shipsSelectHasFocus();
    bool loadedShipsSelectHasFocus();
    void validateButtons();
    void validateButtonNonEmptyListBox(wxButton *button, wxListBox *listBox);
    void validateButtonSelectedListBox(wxButton *button, wxListBox *listBox);
    void setMnemonicLabels(bool modifierDown);
    void focusStageSelect();
  private:
    std::string condenseIfNecessary(std::string s);
};

class NewMatchEventFilter : public wxEventFilter {
  NewMatchDialog *newMatchDialog_;

  public:
    NewMatchEventFilter(NewMatchDialog *newMatchDialog);
    ~NewMatchEventFilter();
    virtual int FilterEvent(wxEvent& event);
};

#endif
