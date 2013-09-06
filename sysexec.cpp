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

#include <wx/wx.h>
#include <string>
#include "filemanager.h"
#include "sysexec.h"

SystemExecutor::SystemExecutor() {
  fileManager_ = new FileManager();
}

SystemExecutor::~SystemExecutor() {
  delete fileManager_;
}

void SystemExecutor::browseDirectory(const char *dir) {
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

void SystemExecutor::openHtmlFile(const char *file) {
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
