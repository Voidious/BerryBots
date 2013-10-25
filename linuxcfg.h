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

#ifndef LINUX_CFG_H
#define LINUX_CFG_H

#include <string.h>

#define SETTINGS_FILENAME  "berrybots/config"

class LinuxCfg {
  std::string stagesDir_;
  std::string shipsDir_;
  std::string runnersDir_;
  std::string cacheDir_;
  std::string tmpDir_;
  std::string replaysDir_;
  std::string apidocPath_;
  std::string samplesVersion_;
  bool aaDisabled_;

  public:
    LinuxCfg();
    std::string stagesDir();
    std::string shipsDir();
    std::string runnersDir();
    std::string cacheDir();
    std::string tmpDir();
    std::string replaysDir();
    std::string apidocPath();
    bool aaDisabled();
    bool isSaved();
    bool loadSettings();
    void setRootDir(std::string newRootDir);
    bool chooseNewRootDir();
    void save();
  private:
    void loadSetting(std::string key, std::string value);
    std::string configDir();
    std::string userHomeDir();
    std::string settingsPath();
    void copyAllAppFiles(std::string srcDir, bool forceAll);
    void copyAppFiles(std::string srcDir, std::string targetDir, bool force,
                      bool &overwrite, bool &asked);
    void copySamplesOrSkip();
};

#endif
