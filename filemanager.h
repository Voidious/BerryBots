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

#include <exception>
#include "bblua.h"
#include "zipper.h"

extern "C" {
  #include "lua.h"
}

class PackagingListener {
  public:
    virtual void packagingComplete(char **sourceFiles, int numFiles, bool nosrc,
                                   const char *destinationFile) = 0;
    virtual ~PackagingListener() {};
};

class FileNotFoundException : public std::exception {
  char *message_;
  
  public:
    FileNotFoundException(const char *filename);
    ~FileNotFoundException() throw();
    virtual const char* what() const throw();
};

class InvalidLuaFilenameException : public std::exception {
  char *message_;
  
  public:
    InvalidLuaFilenameException(const char *filename);
    ~InvalidLuaFilenameException() throw();
    virtual const char* what() const throw();
};

class LuaException : public std::exception {
  char *message_;
public:
  LuaException(const char *details);
  ~LuaException() throw();
  virtual const char* what() const throw();
};

class FileManager {
  Zipper *zipper_;
  PackagingListener *packagingListener_;
  public:
    FileManager(Zipper *zipper);
    ~FileManager();
    void setListener(PackagingListener *packagingListener);
    void loadStageFile(const char *filename, char **stageDir,
        char **stageDirFilename, char **stageCwd, const char *cacheDir)
        throw (FileNotFoundException*);
    void loadBotFile(const char *filename, char **botDir, char **botDirFilename,
        char **botCwd, const char *cacheDir) throw (FileNotFoundException*);
    bool isLuaFilename(const char *filename);
    bool isZipFilename(const char *filename);
    void packageStage(const char *stageArg, const char *version,
        const char *cacheDir, const char *tmpDir, bool nosrc)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*);
    void packageBot(char *botArg, const char *version, const char *cacheDir,
        const char *tmpDir, bool nosrc)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*);
    void saveBytecode(char *srcFile, char *outputFile, char *luaCwd)
        throw (LuaException*);
    static char* getAbsoluteFilename(const char *dir, const char *filename);
    static char* parseDir(const char *dirAndFilename);
  private:
    char* loadUserLuaFilename(char *userDirPath, const char *metaFilename)
        throw (FileNotFoundException*);
    static void sliceString(char *filename, long start, long rest);
    void loadUserFile(const char *srcFilename, char **userDir,
        char **userFilename, char **userCwd, const char *metaFilename,
        const char *cacheDir) throw (FileNotFoundException*);
    bool hasExtension(const char *filename, const char *extension);
    void packageCommon(lua_State *userState, char *userDir, char *userFilename,
        char *luaCwd, const char *version, const char *metaFilename,
        int prevFiles, int numFiles, int filesCmdLen, char **packFilenames,
        const char *tmpDir, bool nosrc)
        throw (InvalidLuaFilenameException*, LuaException*);
    void crawlFiles(lua_State *L, const char *startFile)
        throw (InvalidLuaFilenameException*, LuaException*);
    bool fileExists(const char *filename);
    void mkdir(const char *filename);
    char* parseFilename(const char *dirAndFilename);
    void createDirIfNecessary(const char *dir);
    void mkdirIfNecessary(char *dir);
    void checkLuaFilename(const char *filename)
        throw (InvalidLuaFilenameException*);
    void throwForLuaError(lua_State *L, const char *formatString)
        throw (LuaException*);
};

#endif
