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
#include <string.h>
#include "filemanager.h"
#include "basedir.h"

bool isConfigured() {
  return true;
}

std::string getFullPath(const char *relativePath) {
  char *absFilename = FileManager::getAbsFilePath(relativePath);
  std::string fullPath = std::string(absFilename);
  delete absFilename;
  return fullPath;
}

std::string getStagesDir() {
  return getFullPath("stages");
}

std::string getShipsDir() {
  return getFullPath("bots");
}

std::string getCacheDir() {
  return getFullPath("cache");
}

std::string getTmpDir() {
  return getFullPath(".tmp");
}

std::string getApidocPath() {
  return getFullPath("apidoc/index.html");
}

void setRootDir(std::string newRootDir) {
  // no-op
}

void chooseNewRootDir() {
  // no-op
}
