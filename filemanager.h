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

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

extern "C" {
  #include "lua.h"
}

class FileManager {
  public:
    FileManager();
    ~FileManager();
    void loadStageFile(const char *filename, char **stageDir,
                       char **stageDirFilename, char **stageCwd,
                       const char *cacheDir);
    void loadBotFile(const char *filename, char **botDir, char **botDirFilename,
                     char **botCwd, const char *cacheDir);
    bool isLuaFilename(const char *filename);
    bool isZipFilename(const char *filename);
    void checkLuaFilename(const char *filename);
    void packageStage(const char *stageArg, const char *version,
                      const char *cacheDir, const char *tmpDir, bool nosrc);
    void packageBot(char *botArg, char *version, const char *cacheDir,
                    const char *tmpDir, bool nosrc);
  private:
    char* loadUserLuaFilename(char *userDirPath, const char *metaFilename);
    void sliceString(char *filename, long start, long rest);
    char* getAbsoluteFilename(char *dir, char *filename);
    void loadUserFile(const char *srcFilename, char **userDir,
                      char **userFilename, char **userCwd,
                      const char *metaFilename, const char *cacheDir);
    bool hasExtension(const char *filename, const char *extension);
    void packageCommon(lua_State *userState, char *userDir, char *userFilename,
                       char *luaCwd, const char *version,
                       const char *metaFilename, int prevFiles, int numFiles,
                       int prevCmdLen, char **packFilenames, const char *tmpDir,
                       bool nosrc);
    void crawlFiles(lua_State *L, const char *startFile);
    bool fileExists(const char *filename);
    void mkdir(const char *filename);
    char* parseFilename(const char *dirAndFilename);
    char* parseDir(const char *dirAndFilename);
    void createDirIfNecessary(const char *dir);
    void mkdirIfNecessary(char *dir);
};

#endif
