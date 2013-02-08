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

#ifndef GUI_ZIPPER_H
#define GUI_ZIPPER_H

#include <libarchive/archive.h>
#include "zipper.h"

class GuiZipper : public Zipper {
  public:
    GuiZipper();
    ~GuiZipper();
    virtual void packageFiles(const char *outputFile, const char *baseDir,
        char **filenames, int numFiles, const char *absMetaFilename,
        const char *metaFilename);
    virtual void unpackFile(const char *zipFile, const char *outputDir);
  private:
    void packageSingleFile(const char *absFilename, const char *filename,
                           struct archive *a);
    ssize_t copyData(struct archive *archiveRead, struct archive *archiveWrite,
                     const char *userDirPath);
    void checkForErrors(const char *message, struct archive *a, int r);
};

#endif
