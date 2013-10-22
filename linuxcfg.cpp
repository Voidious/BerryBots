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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <wx/wx.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <platformstl/filesystem/readdir_sequence.hpp>
#include "bbconst.h"
#include "ResourcePath.hpp"
#include "filemanager.h"
#include "linuxcfg.h"

LinuxCfg::LinuxCfg() {
  aaDisabled_ = false;
}

std::string LinuxCfg::stagesDir() {
  return stagesDir_;
}

std::string LinuxCfg::shipsDir() {
  return shipsDir_;
}

std::string LinuxCfg::runnersDir() {
  return runnersDir_;
}

std::string LinuxCfg::cacheDir() {
  return cacheDir_;
}

std::string LinuxCfg::tmpDir() {
  return tmpDir_;
}

std::string LinuxCfg::replaysDir() {
  return replaysDir_;
}

std::string LinuxCfg::apidocPath() {
  std::string apidocPath = resourcePath();
  apidocPath.append("apidoc/index.html");
  return apidocPath;
}

bool LinuxCfg::aaDisabled() {
  return aaDisabled_;
}

bool LinuxCfg::isSaved() {
  FileManager fileManager;
  return fileManager.fileExists(settingsPath().c_str());
}

bool LinuxCfg::loadSettings() {
  bool selectRoot = true;
  FileManager fileManager;

  if (fileManager.fileExists(settingsPath().c_str())) {
    char *settingsFile = fileManager.readFile(settingsPath().c_str());
    std::string s(settingsFile);
    size_t pos = 0;
    size_t i;
    while ((i = s.find('\n', pos)) != std::string::npos) {
      std::string line = s.substr(pos, i - pos);
      if (line.length() > 0) {
        size_t j;
        if ((j = line.find('=', 0)) != std::string::npos) {
          std::string key = line.substr(0, j);
          std::string value = line.substr(j + 1, line.length() - j - 1);
          loadSetting(key, value);
        }
      }
      pos = i + 1;
    }
    delete settingsFile;

    if (stagesDir_.length() > 0 && fileManager.fileExists(stagesDir_.c_str())
        && shipsDir_.length() > 0
        && fileManager.fileExists(shipsDir_.c_str())) {
      selectRoot = false;
    }
  }

  bool loaded = (selectRoot ? chooseNewRootDir() : true);
  if (loaded && (samplesVersion_ != SAMPLES_VERSION)) {
    copySamplesOrSkip();
    samplesVersion_ = SAMPLES_VERSION;
    save();
  }

  return loaded;
}

void LinuxCfg::setRootDir(std::string newRootDir) {
  stagesDir_ = shipsDir_ = runnersDir_ = cacheDir_ = tmpDir_ = replaysDir_ =
      newRootDir;
  stagesDir_.append("/stages");
  shipsDir_.append("/bots");
  runnersDir_.append("/runners");
  cacheDir_.append("/cache");
  tmpDir_.append("/.tmp");
  replaysDir_.append("/replays");
  samplesVersion_ = SAMPLES_VERSION;
  copyAllAppFiles(resourcePath(), false);
  save();
}

bool LinuxCfg::chooseNewRootDir() {
  wxDirDialog dialog(NULL, "Select a BerryBots base directory");
  int r = dialog.ShowModal();
  if (r == wxID_OK) {
    wxString rootDir = dialog.GetPath();
    setRootDir(std::string(rootDir.mb_str()));
    return true;
  } else {
    return false;
  }
}

void LinuxCfg::save() {
  std::stringstream settings;
  settings << "stagesDir=" << stagesDir_ << std::endl
           << "shipsDir=" << shipsDir_ << std::endl
           << "runnersDir=" << runnersDir_ << std::endl
           << "cacheDir=" << cacheDir_ << std::endl
           << "tmpDir=" << tmpDir_ << std::endl
           << "replaysDir=" << replaysDir_ << std::endl
           << "samplesVersion=" << samplesVersion_ << std::endl
           << "aaDisabled=" << (aaDisabled_ ? "true" : "false") << std::endl;

  FileManager fileManager;
  fileManager.writeFile(settingsPath().c_str(), settings.str().c_str());
}

void LinuxCfg::copyAllAppFiles(std::string srcDir, bool forceAll) {
  bool overwrite = false;
  bool asked = false;

  std::string stagesSrc = srcDir;
  stagesSrc.append("stages");
  copyAppFiles(stagesSrc, stagesDir_, forceAll, overwrite, asked);

  std::string shipsSrc = srcDir;
  shipsSrc.append("bots");
  copyAppFiles(shipsSrc, shipsDir_, forceAll, overwrite, asked);

  std::string runnersSrc = srcDir;
  runnersSrc.append("runners");
  copyAppFiles(runnersSrc, runnersDir_, forceAll, overwrite, asked);
}

void LinuxCfg::copyAppFiles(std::string srcDir, std::string targetDir,
                            bool force, bool &overwrite, bool &asked) {
  FileManager fileManager;
  fileManager.createDirectoryIfNecessary(targetDir.c_str());

  platformstl::readdir_sequence dir(srcDir.c_str(),
      platformstl::readdir_sequence::files
          | platformstl::readdir_sequence::directories);
  platformstl::readdir_sequence::const_iterator first = dir.begin();
  platformstl::readdir_sequence::const_iterator last = dir.end();
  while (first != last) {
    platformstl::readdir_sequence::const_iterator file = first++;
    char *filename = (char *) *file;
    char *filePath = fileManager.getFilePath(srcDir.c_str(), filename);
    char *targetPath = fileManager.getFilePath(targetDir.c_str(), filename);
    if (fileManager.isDirectory(filePath)) {
      copyAppFiles(std::string(filePath), std::string(targetPath), force,
                   overwrite, asked);
    } else {
      if (fileManager.fileExists(filePath)) {
        bool destFileExists = fileManager.fileExists(targetPath);
        if (destFileExists && !force && !asked) {
          wxMessageDialog overwriteMessage(NULL,
              "Sample ships and stages already exist in this directory. OK to overwrite them?",
              "Overwrite samples?", wxOK | wxCANCEL | wxICON_QUESTION);
          overwriteMessage.SetOKCancelLabels("&OK", "&Skip");
          int r = overwriteMessage.ShowModal();
          if (r == wxID_OK) {
            overwrite = true;
          }
          asked = true;
        }
        bool doCopy = (force || overwrite || !destFileExists);
        if (doCopy) {
          char *fileContents = fileManager.readFile(filePath);
          fileManager.writeFile(targetPath, fileContents);
          delete fileContents;
        }
      }
    }
    delete filePath;
    delete targetPath;
  }
}

void LinuxCfg::copySamplesOrSkip() {
  std::stringstream okToUpdate;
  okToUpdate << "BerryBots samples  have been updated to version "
             << SAMPLES_VERSION
             << ". OK to update the files in your base directory?";
  wxMessageDialog updateDialog(NULL, okToUpdate.str(), "Update samples?",
                                wxOK | wxCANCEL | wxICON_QUESTION);
  updateDialog.SetOKCancelLabels("&OK", "&Skip");
  int r = updateDialog.ShowModal();

  if (r == wxID_OK) {
    std::string srcDir = resourcePath();
    copyAllAppFiles(srcDir, true);

    std::stringstream samplesUpdated;
    samplesUpdated << "BerryBots samples and API docs have been updated to v"
                   << SAMPLES_VERSION << ".";

    wxMessageDialog updatedDialog(NULL, samplesUpdated.str(),
        "Samples updated.", wxOK | wxICON_INFORMATION);
    updatedDialog.ShowModal();
  }
}

void LinuxCfg::loadSetting(std::string key, std::string value) {
  if (key.compare("stagesDir") == 0) {
    stagesDir_ = value;
  } else if (key.compare("shipsDir") == 0) {
    shipsDir_ = value;
  } else if (key.compare("runnersDir") == 0) {
    runnersDir_ = value;
  } else if (key.compare("cacheDir") == 0) {
    cacheDir_ = value;
  } else if (key.compare("tmpDir") == 0) {
    tmpDir_ = value;
  } else if (key.compare("replaysDir") == 0) {
    replaysDir_ = value;
  } else if (key.compare("samplesVersion") == 0) {
    samplesVersion_ = value;
  } else if (key.compare("aaDisabled") == 0) {
    aaDisabled_ = (value.compare("true") == 0);
  }
}

std::string LinuxCfg::userHomeDir() {
  char *homeEnv = getenv("HOME");
  std::string userHomeDir;
  if (homeEnv == NULL) {
    struct passwd *pw = getpwuid(getuid());
    userHomeDir.append(pw->pw_dir);
  } else {
    userHomeDir.append(homeEnv);
  }
  return userHomeDir;
}

std::string LinuxCfg::settingsPath() {
  std::string settingsPath(userHomeDir());
  settingsPath.append("/");
  settingsPath.append(SETTINGS_FILENAME);
  return settingsPath;
}
