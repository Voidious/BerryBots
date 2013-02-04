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

#include <iostream>
#include <sstream>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include "filemanager.h"
#include "tarzipper.h"

TarZipper::TarZipper() {

}

TarZipper::~TarZipper() {

}

void TarZipper::packageFiles(const char *outputFile, const char *baseDir,
    char **filenames, int numFiles, const char *absMetaFilename,
    const char *metaFilename) {
  std::stringstream cmdStream;
  cmdStream << "cd \"" << baseDir << "\"; "
            << "cp \"" << absMetaFilename << "\" ./" << metaFilename << "; "
            << "tar czfv \"" << outputFile << "\"";
  for (int x = 0; x < numFiles; x++) {
    cmdStream << " \"" << filenames[x] << "\"";
  }
  cmdStream << " " << metaFilename << "; rm ./" << metaFilename;
  system(cmdStream.str().c_str());
}

void TarZipper::unpackFile(const char *zipFile, const char *outputDir) {
  int extractCmdLen = (int) (8 + strlen(zipFile) + 4 + strlen(outputDir));
  char *extractCmd = new char[extractCmdLen + 1];
  sprintf(extractCmd, "tar xfv %s -C %s", zipFile, outputDir);
  system(extractCmd);
  delete extractCmd;
}
