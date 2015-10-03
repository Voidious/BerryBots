/*
  Copyright (C) 2012-2015 - Voidious

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

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/iconbndl.h>
#include "bbwx.h"
#include "basedir.h"
#include "ResourcePath.hpp"
#include "filemanager.h"
#include "newmatch.h"

NewMatchDialog::NewMatchDialog(NewMatchListener *listener,
    MenuBarMaker *menuBarMaker) : wxFrame(NULL, NEW_MATCH_ID, "New Match",
        wxPoint(50, 50), wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
  listener_ = listener;
  menuBarMaker_ = menuBarMaker;

#ifdef __WINDOWS__
  SetIcon(wxIcon(resourcePath() + BERRYBOTS_ICO, wxBITMAP_TYPE_ICO));

  // The 8-9 point default font size in Windows is much smaller than Mac/Linux.
  wxFont windowFont = GetFont();
  if (windowFont.GetPointSize() <= 9) {
    SetFont(windowFont.Larger());
  }
#elif defined(__WXGTK__)
  SetIcon(wxIcon(resourcePath() + BBICON_128, wxBITMAP_TYPE_PNG));
#endif

#ifndef __WINDOWS__
  // Bizarrely, it's impossible to add any padding between the bitmap and the
  // edge of the button on Windows. It's ugly enough to just not set a bitmap.
  helpBitmap_ = loadBitmapIcon(HELP_ICON_128, 24);
  folderHomeBitmap_ = loadBitmapIcon(FOLDER_HOME_128, 24);
  wxBitmap folderOpenBitmap = loadBitmapIcon(FOLDER_OPEN_128, 24);
#endif

  mainPanel_ = new wxPanel(this);
  mainSizer_ = new wxBoxSizer(wxHORIZONTAL);
  mainSizer_->Add(mainPanel_);
  borderSizer_ = new wxBoxSizer(wxHORIZONTAL);
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 5, 5);
  wxBoxSizer *stageLabelSizer = new wxBoxSizer(wxHORIZONTAL);
  stageLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "Stage:");
  stageLabelSizer->Add(stageLabel_, 0, wxALIGN_LEFT);
  stageLabelSizer->AddSpacer(27);
  previewLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "<space> to preview");
  stageLabelSizer->Add(previewLabel_);
  wxBoxSizer *stageSizer = new wxBoxSizer(wxVERTICAL);
  stageSizer->Add(stageLabelSizer);
  stageSizer->AddSpacer(3);
  stageSelect_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                               wxSize(275, 225), 0, NULL, wxLB_SORT);
  stageSizer->Add(stageSelect_, 0, wxALIGN_LEFT);
  gridSizer->Add(stageSizer, 0, wxALIGN_LEFT);

  wxBoxSizer *dirsSizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel_);
  browseShipsButton_ = new wxButton(mainPanel_, wxID_ANY, "Ships",
                                    wxDefaultPosition, wxSize(105, 36));
#ifndef __WINDOWS__
  // Don't set impossible to align bitmap on Windows.
  browseShipsButton_->SetBitmap(folderOpenBitmap);
#endif
  dirsSizer->Add(browseShipsButton_);
  shipsDirLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                             wxEmptyString);
  shipsDirLabel_->SetFont(GetFont().Smaller());
  dirsSizer->AddSpacer(3);
  dirsSizer->Add(shipsDirLabel_, 0 ,wxALIGN_BOTTOM);
  dirsSizer->AddStretchSpacer(2);
  browseStagesButton_ = new wxButton(mainPanel_, wxID_ANY, "Stages",
                                     wxDefaultPosition, wxSize(105, 36));
#ifndef __WINDOWS__
  // Don't set impossible to align bitmap on Windows.
  browseStagesButton_->SetBitmap(wxBitmap(folderOpenBitmap));
#endif
  dirsSizer->Add(browseStagesButton_);

  stagesDirLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                              wxEmptyString);
  stagesDirLabel_->SetFont(GetFont().Smaller());
  dirsSizer->AddSpacer(3);
  dirsSizer->Add(stagesDirLabel_, 0 ,wxALIGN_BOTTOM);
  onSetBaseDirs();

  dirsSizer->AddStretchSpacer(3);
  browseApidocsButton_ = new wxButton(mainPanel_, wxID_ANY, "&API Docs",
                                      wxDefaultPosition, wxSize(145, 36));
#ifndef __WINDOWS__
  // Don't set impossible to align bitmap on Windows.
  browseApidocsButton_->SetBitmap(helpBitmap_);
#endif
  dirsSizer->Add(browseApidocsButton_);

#ifndef __WINDOWS__
  // Still using cwd as base dir on Windows, for now.
  folderButton_ = new wxButton(mainPanel_, wxID_ANY, "&Base Directory",
                               wxDefaultPosition, wxSize(145, 36));
  folderButton_->SetBitmap(folderHomeBitmap_);
  dirsSizer->AddStretchSpacer(3);
  dirsSizer->Add(folderButton_);
  Connect(folderButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onChangeBaseDir));
#endif

  gridSizer->Add(dirsSizer, 0, wxEXPAND | wxLEFT, 8);

  shipsLabel_ = new wxStaticText(mainPanel_, wxID_ANY, "Ships:");
  shipsSelect_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                               wxSize(275, 225), 0, NULL,
                               wxLB_EXTENDED | wxLB_SORT);
  wxBoxSizer *shipsSizer = new wxBoxSizer(wxVERTICAL);
  shipsSizer->Add(shipsLabel_, 0, wxALIGN_LEFT);
  shipsSizer->AddSpacer(3);
  shipsSizer->Add(shipsSelect_, 0, wxALIGN_LEFT);
  gridSizer->Add(shipsSizer, 0, wxALIGN_LEFT);

  wxBoxSizer *buttonsLoadedShipsSizer = new wxBoxSizer(wxHORIZONTAL);
  addArrow_ = new wxButton(mainPanel_, wxID_ANY, ">>", wxDefaultPosition,
                           wxDefaultSize);
  removeArrow_ = new wxButton(mainPanel_, wxID_ANY, "<<", wxDefaultPosition,
                              wxDefaultSize);
  clearButton_ = new wxButton(mainPanel_, wxID_ANY, "C&lear", wxDefaultPosition,
                              wxDefaultSize);

  wxBoxSizer *shipButtonsSizer = new wxBoxSizer(wxVERTICAL);
  shipButtonsSizer->AddStretchSpacer(1);
  shipButtonsSizer->Add(addArrow_, 0, wxALIGN_CENTER);
  shipButtonsSizer->AddSpacer(5);
  shipButtonsSizer->Add(removeArrow_, 0, wxALIGN_CENTER);
  shipButtonsSizer->AddSpacer(5);
  shipButtonsSizer->Add(clearButton_, 0, wxALIGN_CENTER);
  shipButtonsSizer->AddStretchSpacer(1);
#ifdef __WXOSX__
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                    "\u2318 hotkeys");
#else
  keyboardLabel_ = new wxStaticText(mainPanel_, wxID_ANY,
                                    "ALT hotkeys");
#endif
  shipButtonsSizer->Add(keyboardLabel_, 0, wxALIGN_CENTER | wxALIGN_BOTTOM);
  buttonsLoadedShipsSizer->Add(shipButtonsSizer, 0, wxALIGN_CENTER | wxEXPAND);

  loadedShipsSelect_ = new wxListBox(mainPanel_, wxID_ANY, wxDefaultPosition,
                                     wxSize(275, 225), 0, NULL, wxLB_EXTENDED);
  buttonsLoadedShipsSizer->AddSpacer(5);
  buttonsLoadedShipsSizer->Add(loadedShipsSelect_, 0,
                              wxALIGN_BOTTOM | wxALIGN_RIGHT);
  gridSizer->Add(buttonsLoadedShipsSizer, 0, wxALIGN_BOTTOM);

  refreshButton_ = new wxButton(mainPanel_, wxID_REFRESH, "    &Refresh    ");
  gridSizer->Add(refreshButton_, 0, wxALIGN_LEFT);

#ifdef __WXOSX__
  // If we start the first match via the wxWidgets 2.9.5+ mnemonics on Mac OS X,
  // the app will crash the first time we start a match with a mouse click. We
  // already have hand-rolled mnemonics for OS X, so use those instead. This is
  // similar to how we need to be careful about initializing SFML windows before
  // any wxWidgets operations in GuiManager.
  startButton_ = new wxButton(mainPanel_, wxID_ANY, "    Start Match!    ",
                              wxDefaultPosition, wxDefaultSize);
#else
  startButton_ = new wxButton(mainPanel_, wxID_ANY, "    Start &Match!    ",
                              wxDefaultPosition, wxDefaultSize);
#endif
  browseApidocsButton_->MoveAfterInTabOrder(browseShipsButton_);
  gridSizer->Add(startButton_, 0, wxALIGN_RIGHT);
  borderSizer_->Add(gridSizer, 0, wxALL, 12);
  mainPanel_->SetSizerAndFit(borderSizer_);
  SetSizerAndFit(mainSizer_);

  numStages_ = numShips_ = numLoadedShips_ = 0;
  menusInitialized_ = false;
  validateButtons();

  Connect(NEW_MATCH_ID, wxEVT_ACTIVATE,
          wxActivateEventHandler(NewMatchDialog::onActivate));
  Connect(NEW_MATCH_ID, wxEVT_CLOSE_WINDOW,
          wxCommandEventHandler(NewMatchDialog::onClose));
  Connect(addArrow_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddShips));
  Connect(removeArrow_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveShips));
  Connect(clearButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onClearLoadedShips));
  Connect(startButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onStartMatch));
  Connect(shipsSelect_->GetId(), wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onAddShips));
  Connect(loadedShipsSelect_->GetId(), wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,
          wxCommandEventHandler(NewMatchDialog::onRemoveShips));
  Connect(refreshButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onRefreshFiles));
  Connect(stageSelect_->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(NewMatchDialog::onSelectStage));
  Connect(shipsSelect_->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(NewMatchDialog::onSelectShip));
  Connect(loadedShipsSelect_->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(NewMatchDialog::onSelectLoadedShip));
  Connect(browseApidocsButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onBrowseApidocs));
#ifdef __WINDOWS
  // On Windows XP, not redrawing the game while interrupted (dialog shown)
  // creates visual artifacts, so manually redraw it on update UI in Windows.
  // The same kills the framerate in Linux/GTK. Either way works on Mac/Cocoa.
  Connect(this->GetId(), wxEVT_UPDATE_UI,
          wxUpdateUIEventHandler(NewMatchDialog::onUpdateUi));
#endif
  
#if defined(__WXOSX__) || defined(__LINUX__) || defined(__WINDOWS__)
  browseStagesButton_->MoveAfterInTabOrder(startButton_);
  browseShipsButton_->MoveAfterInTabOrder(browseStagesButton_);
  browseApidocsButton_->MoveAfterInTabOrder(browseShipsButton_);
  Connect(browseStagesButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onBrowseStages));
  Connect(browseShipsButton_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewMatchDialog::onBrowseShips));
#endif

#ifdef __WXOSX__
  folderButton_->MoveAfterInTabOrder(browseShipsButton_);
  browseApidocsButton_->MoveAfterInTabOrder(folderButton_);
#endif

  eventFilter_ = new NewMatchEventFilter(this);
  this->GetEventHandler()->AddFilter(eventFilter_);
}

wxBitmap NewMatchDialog::loadBitmapIcon(std::string path, int size) {
  double backingScale = getBackingScaleFactor();
  wxImage img;
  img.LoadFile(std::string(resourcePath() + path).c_str());
  img.Rescale(backingScale * size, backingScale * size, wxIMAGE_QUALITY_HIGH);
  wxBitmap bitmap;
  bitmap.CreateScaled(size, size, wxBITMAP_SCREEN_DEPTH, backingScale);
  wxMemoryDC dc(bitmap);
  double logicalScale = (backingScale > 1) ? (1.0 / backingScale) : 1;
  dc.SetLogicalScale(logicalScale, logicalScale);
  dc.DrawBitmap(wxBitmap(img), 0, 0);
  return bitmap;
}

NewMatchDialog::~NewMatchDialog() {
  this->GetEventHandler()->RemoveFilter(eventFilter_);
  delete eventFilter_;
}

void NewMatchDialog::clearStages() {
  stageSelect_->Clear();
  numStages_ = 0;
}

void NewMatchDialog::addStage(char *stage) {
  stageSelect_->Append(wxString(stage));
  numStages_++;
  if (stageSelect_->GetCount() > 0) {
    stageSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::clearShips() {
  shipsSelect_->Clear();
  numShips_ = 0;
}

void NewMatchDialog::addShip(char *ship) {
  shipsSelect_->Append(wxString(ship));
  numShips_++;
  if (shipsSelect_->GetCount() > 0) {
    shipsSelect_->SetFirstItem(0);
  }
}

void NewMatchDialog::onActivate(wxActivateEvent &event) {
  if (!menusInitialized_) {
    this->SetMenuBar(menuBarMaker_->getNewMenuBar());
    menusInitialized_ = true;
    Fit();
  }
}

void NewMatchDialog::onClose(wxCommandEvent &event) {
  listener_->onClose();
}

void NewMatchDialog::onAddShips(wxCommandEvent &event) {
  addSelectedShips();
}

void NewMatchDialog::addSelectedShips() {
  wxArrayInt selectedShips;
  shipsSelect_->GetSelections(selectedShips);
  wxArrayInt::const_iterator first = selectedShips.begin();
  wxArrayInt::const_iterator last = selectedShips.end();
  while (first != last) {
    int shipIndex = *first++;
    loadedShipsSelect_->Insert(shipsSelect_->GetString(shipIndex),
                               numLoadedShips_++);
  }
  validateButtons();
}

void NewMatchDialog::onRemoveShips(wxCommandEvent &event) {
  removeSelectedLoadedShips();
}

void NewMatchDialog::removeSelectedLoadedShips() {
  wxArrayInt selectedShips;
  loadedShipsSelect_->GetSelections(selectedShips);
  wxArrayInt::const_iterator first = selectedShips.begin();
  wxArrayInt::const_iterator last = selectedShips.end();
  int removed = 0;
  while (first != last) {
    int shipIndex = *first++;
    loadedShipsSelect_->Delete(shipIndex - (removed++));
    numLoadedShips_--;
  }
  validateButtons();
}

void NewMatchDialog::removeStaleLoadedShips() {
  if (numLoadedShips_ > 0) {
    for (int x = 0; x < numLoadedShips_; x++) {
      wxString loadedShip = loadedShipsSelect_->GetString(x);
      if (shipsSelect_->wxItemContainerImmutable::FindString(loadedShip)
              == wxNOT_FOUND) {
        loadedShipsSelect_->Delete(x);
        x--;
        numLoadedShips_--;
      }
    }
  }
  validateButtons();
}

void NewMatchDialog::onClearLoadedShips(wxCommandEvent &event) {
  clearLoadedShips();
}

void NewMatchDialog::clearLoadedShips() {
  loadedShipsSelect_->Clear();
  numLoadedShips_ = 0;
  validateButtons();
}

void NewMatchDialog::onStartMatch(wxCommandEvent &event) {
  startMatch();
}

void NewMatchDialog::startMatch() {
  if (listener_ != 0) {
    wxArrayInt selectedStageIndex;
    stageSelect_->GetSelections(selectedStageIndex);
    if (numLoadedShips_ > 0 && selectedStageIndex.Count() > 0) {
      int numStartShips = numLoadedShips_;
      char** ships = new char*[numStartShips];
      for (int x = 0; x < numStartShips; x++) {
        wxString loadedShip = loadedShipsSelect_->GetString(x);
        char *ship = new char[loadedShip.length() + 1];
#ifdef __WINDOWS__
        strcpy(ship, loadedShip.c_str());
#else
        strcpy(ship, loadedShip.fn_str());
#endif
        ships[x] = ship;
      }
      
      wxString selectedStage =
          stageSelect_->GetString(*(selectedStageIndex.begin()));
      char *stage = new char[selectedStage.length() + 1];
#ifdef __WINDOWS__
      strcpy(stage, selectedStage.c_str());
#else
      strcpy(stage, selectedStage.fn_str());
#endif
      
      listener_->startMatch(stage, ships, numStartShips);
      
      for (int x = 0; x < numStartShips; x++) {
        delete ships[x];
      }
      delete ships;
      delete stage;
    }
  }
}

void NewMatchDialog::onRefreshFiles(wxCommandEvent &event) {
  refreshFiles();
}

void NewMatchDialog::refreshFiles() {
  // TODO: reselect previously selected ships and stage
  listener_->refreshFiles();
  validateButtons();
}

void NewMatchDialog::onBrowseStages(wxCommandEvent &event) {
  wxCommandEvent menuEvent(wxEVT_COMMAND_MENU_SELECTED, BROWSE_STAGES_MENU_ID);
  menuEvent.SetEventObject(this);
  ProcessWindowEvent(menuEvent);
}

void NewMatchDialog::onBrowseShips(wxCommandEvent &event) {
  wxCommandEvent menuEvent(wxEVT_COMMAND_MENU_SELECTED, BROWSE_SHIPS_MENU_ID);
  menuEvent.SetEventObject(this);
  ProcessWindowEvent(menuEvent);
}

void NewMatchDialog::onBrowseApidocs(wxCommandEvent &event) {
  wxCommandEvent menuEvent(wxEVT_COMMAND_MENU_SELECTED,
                           BROWSE_API_DOCS_MENU_ID);
  menuEvent.SetEventObject(this);
  ProcessWindowEvent(menuEvent);
}

void NewMatchDialog::onChangeBaseDir(wxCommandEvent &event) {
  wxCommandEvent menuEvent(wxEVT_COMMAND_MENU_SELECTED,
                           CHANGE_BASE_DIR_MENU_ID);
  menuEvent.SetEventObject(this);
  ProcessWindowEvent(menuEvent);
}

void NewMatchDialog::onEscape() {
  listener_->onEscape();
}

void NewMatchDialog::onSelectStage(wxUpdateUIEvent &event) {
  validateButtons();
}

void NewMatchDialog::onSelectShip(wxUpdateUIEvent &event) {
  validateButtons();
}

void NewMatchDialog::onSelectLoadedShip(wxUpdateUIEvent &event) {
  validateButtons();
}

void NewMatchDialog::onUpdateUi(wxUpdateUIEvent &event) {
  listener_->onUpdateUi();
}

void NewMatchDialog::onSetBaseDirs() {
  stagesDirLabel_->SetLabelText(condenseIfNecessary(getStagesDir()));
  shipsDirLabel_->SetLabelText(condenseIfNecessary(getShipsDir()));
}

std::string NewMatchDialog::condenseIfNecessary(std::string s) {
  if (s.size() > 55) {
    std::string cs;
    cs.append(s.substr(0, 15));
    cs.append("...");
    cs.append(s.substr(s.size() - 37, 37));
    return cs;
  } else {
    return s;
  }
}

void NewMatchDialog::previewSelectedStage() {
  wxArrayInt selectedStageIndex;
  stageSelect_->GetSelections(selectedStageIndex);
  if (selectedStageIndex.Count() > 0) {
    previewStage(*(selectedStageIndex.begin()));
  }
}

void NewMatchDialog::previewNextStage() {
  wxArrayInt selectedStageIndex;
  stageSelect_->GetSelections(selectedStageIndex);
  if (selectedStageIndex.Count() > 0) {
    int i = *(selectedStageIndex.begin());
    if (i < stageSelect_->GetCount() - 1) {
      previewStage(i + 1);
    }
  }
}

void NewMatchDialog::previewPreviousStage() {
  wxArrayInt selectedStageIndex;
  stageSelect_->GetSelections(selectedStageIndex);
  if (selectedStageIndex.Count() > 0) {
    int i = *(selectedStageIndex.begin());
    if (i > 0) {
      previewStage(i - 1);
    }
  }
}

void NewMatchDialog::previewStage(int i) {
  wxString stageName = stageSelect_->GetString(i);
  char *stage = new char[stageName.length() + 1];
#ifdef __WINDOWS__
  strcpy(stage, stageName.c_str());
#else
  strcpy(stage, stageName.fn_str());
#endif
  listener_->previewStage(stage);
  delete stage;
}

bool NewMatchDialog::stageSelectHasFocus() {
  return stageSelect_->HasFocus();
}

bool NewMatchDialog::shipsSelectHasFocus() {
  return shipsSelect_->HasFocus();
}

bool NewMatchDialog::loadedShipsSelectHasFocus() {
  return loadedShipsSelect_->HasFocus();
}

void NewMatchDialog::validateButtons() {
  if (numLoadedShips_ > 0) {
    validateButtonSelectedListBox(startButton_, stageSelect_);
  } else {
    startButton_->Disable();
  }

  validateButtonSelectedListBox(addArrow_, shipsSelect_);
  validateButtonSelectedListBox(removeArrow_, loadedShipsSelect_);
  validateButtonNonEmptyListBox(clearButton_, loadedShipsSelect_);
}

void NewMatchDialog::validateButtonNonEmptyListBox(wxButton *button,
                                                   wxListBox *listBox) {
  if (listBox->GetCount() > 0) {
    button->Enable();
  } else {
    button->Disable();
  }
}

void NewMatchDialog::validateButtonSelectedListBox(wxButton *button,
                                                   wxListBox *listBox) {
  wxArrayInt selectedIndex;
  listBox->GetSelections(selectedIndex);
  if (selectedIndex.Count() > 0) {
    button->Enable();
  } else {
    button->Disable();
  }
}

void NewMatchDialog::setMnemonicLabels(bool modifierDown) {
  // TODO: I'd rather it look like the button was pressed when you hit the
  //       shortcut, if possible. For now having trouble figuring out the
  //       wxButton::Command() call.
  if (modifierDown) {
#ifdef __WXOSX__
    clearButton_->SetLabel("C&lear \u2318L");
    refreshButton_->SetLabel("&Refresh \u2318R");
    startButton_->SetLabel("Start Match \u2318M");
    browseApidocsButton_->SetLabel("&API Docs  \u2318A");
    folderButton_->SetLabel("&Base Dir   \u2318B");
#else
    clearButton_->SetLabel("C&lear  alt-L");
    refreshButton_->SetLabel("&Refresh  alt-R");
    startButton_->SetLabel("Start &Match!  alt-M");
    browseApidocsButton_->SetLabel("&API Docs  alt-A");
#ifndef __WINDOWS__
    folderButton_->SetLabel("&Base Dir  alt-B");
#endif
#endif
  } else {
    clearButton_->SetLabel("C&lear");
    refreshButton_->SetLabel("    &Refresh    ");
    browseApidocsButton_->SetLabel("&API Docs");
#ifndef __WINDOWS__
    folderButton_->SetLabel("&Base Directory");
#endif
#ifdef __WXOSX__
    startButton_->SetLabel("    Start Match!    ");
#else
    startButton_->SetLabel("    Start &Match!    ");
#endif
  }
#ifndef __WINDOWS__
  browseApidocsButton_->SetBitmap(helpBitmap_);
  folderButton_->SetBitmap(folderHomeBitmap_);
#endif
}

void NewMatchDialog::focusStageSelect() {
  stageSelect_->SetFocus();
}

void NewMatchDialog::selectStage(const char *name) {
  int numStrings = stageSelect_->GetCount();
  for (int x = 0; x < numStrings; x++) {
    wxString itemName = stageSelect_->GetString(x);
    if (strcmp(itemName.c_str(), name) == 0) {
      stageSelect_->Select(x);
      stageSelect_->SetFirstItem(x);
      break;
    }
  }
}

char** NewMatchDialog::getStageNames() {
  return getSelectStrings(stageSelect_);
}

char** NewMatchDialog::getShipNames() {
  return getSelectStrings(shipsSelect_);
}

char** NewMatchDialog::getSelectStrings(wxListBox *listBox) {
  int numStrings = listBox->GetCount();
  char** strings = new char*[numStrings];
  for (int x = 0; x < numStrings; x++) {
    wxString itemName = listBox->GetString(x);
    strings[x] = new char[itemName.size() + 1];
    strcpy(strings[x], itemName.c_str());
  }
  return strings;
}

NewMatchEventFilter::NewMatchEventFilter(NewMatchDialog *newMatchDialog) {
  newMatchDialog_ = newMatchDialog;
}

NewMatchEventFilter::~NewMatchEventFilter() {

}

int NewMatchEventFilter::FilterEvent(wxEvent& event) {
  bool modifierDown = false;
  wxKeyEvent *keyEvent = ((wxKeyEvent*) &event);
#if defined(__WXOSX__)
  modifierDown = keyEvent->ControlDown();
#elif defined(__WINDOWS__)
  modifierDown = keyEvent->AltDown();
#endif

  const wxEventType type = event.GetEventType();
  if (type == wxEVT_KEY_DOWN && newMatchDialog_->IsActive()) {
    newMatchDialog_->setMnemonicLabels(modifierDown);
    int keyCode = keyEvent->GetKeyCode();
    if (keyCode == WXK_ESCAPE
        || (keyEvent->GetUnicodeKey() == 'W' && keyEvent->ControlDown())) {
      newMatchDialog_->onEscape();
      return Event_Processed;
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_RETURN)
               && newMatchDialog_->shipsSelectHasFocus()) {
      newMatchDialog_->addSelectedShips();
      return Event_Processed;
    } else if ((keyCode == WXK_SPACE || keyCode == WXK_BACK)
               && (newMatchDialog_->loadedShipsSelectHasFocus())) {
      newMatchDialog_->removeSelectedLoadedShips();
      return Event_Processed;
    } else if (keyCode == WXK_SPACE && newMatchDialog_->stageSelectHasFocus()) {
      newMatchDialog_->previewSelectedStage();
      return Event_Processed;
#ifdef __WXOSX__
    // Manually simulate the mnemonic for Start Match button to avoid weird
    // SFML window destructor crashes.
    } else if (keyEvent->GetUnicodeKey() == 'M' && modifierDown) {
      newMatchDialog_->startMatch();
      return Event_Processed;
#endif
    }
  }

  if (type == wxEVT_KEY_UP) {
    newMatchDialog_->setMnemonicLabels(modifierDown);
  } else if (type == wxEVT_KILL_FOCUS) {
    newMatchDialog_->setMnemonicLabels(false);
  }

  return Event_Skip;
}
