/*
  Copyright (C) 2013-2015 - Voidious

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

#include <stdlib.h>
#include "basedir.h"
#include "linuxcfg.h"

LinuxCfg *cfg = 0;

bool loadCfg() {
  if (cfg == 0) {
    cfg = new LinuxCfg();
  }
  return cfg->loadSettings();
}

bool isConfigured() {
  if (cfg == 0) {
    cfg = new LinuxCfg();
  }
  return cfg->isSaved();
}

std::string getStagesDir() {
  if (loadCfg()) {
    return cfg->stagesDir();
  } else {
    exit(0);
  }
}

std::string getShipsDir() {
  if (loadCfg()) {
    return cfg->shipsDir();
  } else {
    exit(0);
  }
}

std::string getRunnersDir() {
  if (loadCfg()) {
    return cfg->runnersDir();
  } else {
    exit(0);
  }
}

std::string getCacheDir() {
  if (loadCfg()) {
    return cfg->cacheDir();
  } else {
    exit(0);
  }
}

std::string getTmpDir() {
  if (loadCfg()) {
    return cfg->tmpDir();
  } else {
    exit(0);
  }
}

std::string getReplaysDir() {
  if (loadCfg()) {
    return cfg->replaysDir();
  } else {
    exit(0);
  }
}

std::string getApidocPath() {
  if (loadCfg()) {
    return cfg->apidocPath();
  } else {
    exit(0);
  }
}

void setRootDir(std::string newRootDir) {
  loadCfg();
  cfg->setRootDir(newRootDir);
}

void chooseNewRootDir() {
  loadCfg();
  cfg->chooseNewRootDir();
}

bool isAaDisabled() {
  if (loadCfg()) {
    return cfg->aaDisabled();
  } else {
    exit(0);
  }
}

double getBackingScaleFactor() {
  return 1;
}
