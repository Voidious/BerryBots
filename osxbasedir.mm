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

#include "basedir.h"
#include "osxcfg.h"
#include <string>
#include <iostream>

OsxCfg *cfg = 0;

bool loadCfg() {
  if (cfg == 0) {
    cfg = [[OsxCfg alloc] init];
  }
  return [cfg loadPlist];
}

bool isConfigured() {
  if (cfg == 0) {
    cfg = [[OsxCfg alloc] init];
  }
  return cfg.hasPlist;
}

std::string getStagesDir(void) {
  if (loadCfg()) {
    return [cfg.stagesDir UTF8String];
  } else {
    exit(0);
  }
}

std::string getShipsDir(void) {
  if (loadCfg()) {
    return [cfg.shipsDir UTF8String];
  } else {
    exit(0);
  }
}

std::string getCacheDir(void) {
  if (loadCfg()) {
    return [cfg.cacheDir UTF8String];
  } else {
    exit(0);
  }
}

std::string getTmpDir(void) {
  if (loadCfg()) {
    return [cfg.tmpDir UTF8String];
  } else {
    exit(0);
  }
}

std::string getApidocPath(void) {
  if (loadCfg()) {
    return [cfg.apidocPath UTF8String];
  } else {
    exit(0);
  }
}

void setRootDir(std::string newRootDir) {
  loadCfg();
  [cfg setRootDir:[NSString stringWithUTF8String:newRootDir.c_str()]];
}

void chooseNewRootDir(void) {
  loadCfg();
  [cfg chooseNewRootDir];
}

bool isAaDisabled(void) {
  if (loadCfg()) {
    return cfg.aaDisabled;
  } else {
    exit(0);
  }
}
