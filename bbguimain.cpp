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

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sstream>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <wx/wx.h>
#include <wx/mimetype.h>

#include "bbwx.h"
#include "stage.h"
#include "bbengine.h"
#include "printhandler.h"
#include "guimanager.h"
#include "bbutil.h"
#include "basedir.h"
#include "filemanager.h"

using namespace std;

PrintHandler *printHandler = 0;

class BerryBotsApp : public wxApp {
  GuiManager *guiManager_;
  GuiListener *guiListener_;
  FileManager *fileManager_;

  public:
    virtual bool OnInit();
    virtual void OnQuit(wxCommandEvent &event);
    void quit();
    virtual void OnNewMatch(wxCommandEvent &event);
    virtual void OnPackageShip(wxCommandEvent &event);
    virtual void OnPackageStage(wxCommandEvent &event);
    virtual void OnGameRunner(wxCommandEvent &event);
    virtual void OnErrorConsole(wxCommandEvent &event);
    virtual void MacReopenApp();
    void onChangeBaseDir(wxCommandEvent &event);
    void onBrowseStages(wxCommandEvent &event);
    void onBrowseShips(wxCommandEvent &event);
    void onBrowseRunners(wxCommandEvent &event);
    void onBrowseApidocs(wxCommandEvent &event);
  private:
    void browseDirectory(const char *dir);
    void openHtmlFile(const char *file);
};

class AppGuiListener : public GuiListener {
  BerryBotsApp *app_;
  
  public:
    AppGuiListener(BerryBotsApp *app);
    ~AppGuiListener();
    void onAllWindowsClosed();
};

wxIMPLEMENT_APP(BerryBotsApp);

bool BerryBotsApp::OnInit() {

#ifdef __WXGTK__
  // Needed to load dock icons for wxWidgets dialogs.
  wxImage::AddHandler(new wxPNGHandler);
#endif

#ifdef __WXOSX__
  // On OS X, it complains if we initialize our first SFML window after the
  // wxWidgets windows have set their menu bars or after the base dir selector,
  // so initialize one here first.
  sf::RenderWindow *window = new sf::RenderWindow(
      sf::VideoMode(800, 600), "BerryBots", sf::Style::Default,
      sf::ContextSettings(0, 0, 0, 2, 0));
  delete window;

  // On OS X, if base directory has not been selected yet on OS, throw up an
  // informational dialog with wxWidgets before the file open dialog.
  if (!isConfigured()) {
    std::stringstream configInfo;
    configInfo << "TLDR: Before you can use BerryBots, you need to select a "
               << "base directory. This is where BerryBots will read and write "
               << "files like ships and stages."
               << std::endl << std::endl
               << "Home > Documents > BerryBots is a reasonable choice."
               << std::endl << std::endl
               << "---"
               << std::endl << std::endl
               << "After selecting a directory, subdirectories will be created "
               << "for ships (bots/) and stages (stages/). The sample ships "
               << "and stages will be copied over. Place your own ships and "
               << "stages there, too."
               << std::endl << std::endl
               << "The BerryBots Lua API documentation will be copied into the "
               << "apidoc/ subdirectory."
               << std::endl << std::endl
               << "As needed, BerryBots will also create a cache (cache/) "
               << "subdirectory for extracting packaged ships and stages, as "
               << "well as a temp (.tmp/) subdirectory for working files used "
               << "to package ships and stages."
               << std::endl << std::endl
               << "Have fun!";
    
    wxMessageDialog selectBaseDirMessage(NULL, configInfo.str(),
                                        "BerryBots Setup", wxOK);
    if (selectBaseDirMessage.ShowModal() != wxID_OK) {
      return false;
    }
  }
#endif

  guiListener_ = new AppGuiListener(this);
  guiManager_ = new GuiManager(guiListener_);
  fileManager_ = new FileManager();

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnQuit));
  Connect(NEW_MATCH_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnNewMatch));
  Connect(PACKAGE_SHIP_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageShip));
  Connect(PACKAGE_STAGE_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnPackageStage));
  Connect(GAME_RUNNER_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnGameRunner));
  Connect(ERROR_CONSOLE_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnErrorConsole));
#ifndef __WXOSX__
  Connect(FILE_QUIT_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::OnQuit));
#endif

  Connect(CHANGE_BASE_DIR_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::onChangeBaseDir));
  Connect(BROWSE_SHIPS_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::onBrowseShips));
  Connect(BROWSE_STAGES_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::onBrowseStages));
  Connect(BROWSE_RUNNERS_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::onBrowseRunners));
  Connect(BROWSE_API_DOCS_MENU_ID, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(BerryBotsApp::onBrowseApidocs));
  
  return true;
}

void BerryBotsApp::OnQuit(wxCommandEvent &event) {
  quit();
}

void BerryBotsApp::quit() {
  guiManager_->quit();
  ExitMainLoop();
  delete guiListener_;
  // TODO: delete guiManager_ without clobbering anyone else
}

void BerryBotsApp::OnNewMatch(wxCommandEvent &event) {
  guiManager_->showNewMatchDialog();
  guiManager_->newMatchInitialFocus();
}

void BerryBotsApp::OnPackageShip(wxCommandEvent &event) {
  guiManager_->showPackageShipDialog();
  guiManager_->packageShipInitialFocus();
}

void BerryBotsApp::OnPackageStage(wxCommandEvent &event) {
  guiManager_->showPackageStageDialog();
  guiManager_->packageStageInitialFocus();
}

void BerryBotsApp::OnGameRunner(wxCommandEvent &event) {
  guiManager_->showGameRunnerDialog();
  guiManager_->gameRunnerInitialFocus();
}

void BerryBotsApp::OnErrorConsole(wxCommandEvent &event) {
  guiManager_->showErrorConsole();
}

void BerryBotsApp::MacReopenApp() {
  // No-op - When SFML window is only one open and user switches to app via icon
  // in Mac OS X dock, this stops wxWidgets from opening one of its windows
  // unnecessarily.
}

void BerryBotsApp::onChangeBaseDir(wxCommandEvent &event) {
  chooseNewRootDir();
  guiManager_->reloadBaseDirs();
  guiManager_->loadStages();
  guiManager_->loadShips();
  guiManager_->loadRunners();
}

void BerryBotsApp::onBrowseStages(wxCommandEvent &event) {
  browseDirectory(getStagesDir().c_str());
}

void BerryBotsApp::onBrowseShips(wxCommandEvent &event) {
  browseDirectory(getShipsDir().c_str());
}

void BerryBotsApp::onBrowseRunners(wxCommandEvent &event) {
  browseDirectory(getRunnersDir().c_str());
}

void BerryBotsApp::onBrowseApidocs(wxCommandEvent &event) {
  openHtmlFile(getApidocPath().c_str());
}

void BerryBotsApp::browseDirectory(const char *dir) {
  if (!fileManager_->fileExists(dir)) {
    std::string fileNotFoundString("Directory not found: ");
    fileNotFoundString.append(dir);
    wxMessageDialog cantBrowseMessage(NULL, "Directory not found",
                                      fileNotFoundString, wxOK);
    cantBrowseMessage.ShowModal();
  } else {
#if defined(__WXOSX__)
    ::wxExecute(wxString::Format("open \"%s\"", dir), wxEXEC_ASYNC, NULL);
#elif defined(__LINUX__)
    ::wxExecute(wxString::Format("xdg-open \"%s\"", dir), wxEXEC_ASYNC, NULL);
#elif defined(__WINDOWS__)
    ::wxExecute(wxString::Format("explorer \"%s\"", dir), wxEXEC_ASYNC, NULL);
#else
    wxMessageDialog cantBrowseMessage(NULL, "Couldn't browse directory",
        "Sorry, don't know how to open/browse files on your platform.", wxOK);
    cantBrowseMessage.ShowModal();
#endif
  }
}

void BerryBotsApp::openHtmlFile(const char *file) {
  if (!fileManager_->fileExists(file)) {
    std::string fileNotFoundString("File not found: ");
    fileNotFoundString.append(file);
    wxMessageDialog cantBrowseMessage(NULL, "File not found",
                                      fileNotFoundString, wxOK);
    cantBrowseMessage.ShowModal();
  } else {
    // On Mac OS X, wxFileType::GetOpenCommand always returns Safari instead of
    // the default browser. And what's worse, Safari doesn't load the CSS
    // properly when we open it that way. But we can trust the 'open' command.
#if defined(__WXOSX__)
    ::wxExecute(wxString::Format("open \"%s\"", file), wxEXEC_ASYNC, NULL);
#else
    wxMimeTypesManager *typeManager = new wxMimeTypesManager();
    wxFileType *htmlType = typeManager->GetFileTypeFromExtension(".html");
    wxString openCommand = htmlType->GetOpenCommand(file);
    if (openCommand.IsEmpty()) {
      wxMessageDialog cantBrowseMessage(NULL, "Couldn't open file",
          "Sorry, don't know how to open/browse files on your platform.", wxOK);
      cantBrowseMessage.ShowModal();
    } else {
      ::wxExecute(openCommand, wxEXEC_ASYNC, NULL);
    }
    delete htmlType;
    delete typeManager;
#endif
  }
}

AppGuiListener::AppGuiListener(BerryBotsApp *app) {
  app_ = app;
}

AppGuiListener::~AppGuiListener() {
  
}

void AppGuiListener::onAllWindowsClosed() {
  app_->quit();
}
