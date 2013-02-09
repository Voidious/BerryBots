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

class FileExistsException : public std::exception {
  char *filename_;
  public:
    FileExistsException(const char *filename);
    ~FileExistsException() throw();
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
        char **stageDirFilename, const char *cacheDir)
        throw (FileNotFoundException*);
    void loadBotFile(const char *filename, char **botDir, char **botDirFilename,
        const char *cacheDir) throw (FileNotFoundException*);
    bool isLuaFilename(const char *filename);
    bool isZipFilename(const char *filename);
    void packageStage(const char *stageBaseDir, const char *stageName,
                      const char *version, const char *cacheDir,
                      const char *tmpDir, bool nosrc, bool force)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*, ZipperException*, FileExistsException*);
    void packageBot(const char *botsBaseDir, const char *botName,
                    const char *version, const char *cacheDir,
                    const char *tmpDir, bool nosrc, bool force)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*, ZipperException*, FileExistsException*);
    void saveBytecode(const char *srcFile, const char *outputFile,
                      const char *luaCwd)
        throw (LuaException*);
    void deleteFromCache(const char *cacheDir, const char *filename);
    static char* getFilePath(const char *dir, const char *filename);
    static char* parseDir(const char *dirAndFilename);
    static char* parseFilename(const char *dirAndFilename);
    static bool isDirectory(const char *filePath);
  private:
    char* loadUserLuaFilename(char *userDirPath, const char *metaFilename)
        throw (FileNotFoundException*);
    static void sliceString(char *filename, long start, long rest);
    void loadUserFile(const char *srcFilename, char **userDir,
        char **userFilename, const char *metaFilename,
        const char *cacheDir) throw (FileNotFoundException*);
    bool hasExtension(const char *filename, const char *extension);
    void packageCommon(lua_State *userState, const char *userDir,
        const char *userFilename, const char *version, const char *metaFilename,
        int prevFiles, int numFiles, char **packFilenames, const char *tmpDir,
        bool nosrc, bool force)
        throw (InvalidLuaFilenameException*, LuaException*, ZipperException*,
               FileExistsException*);
    void crawlFiles(lua_State *L, const char *startFile)
        throw (InvalidLuaFilenameException*, LuaException*);
    bool fileExists(const char *filename);
    void createDirectory(const char *filename);
    void createDirectoryIfNecessary(const char *dir);
    void recursiveDelete(const char *fileToDelete);
    void checkLuaFilename(const char *filename)
        throw (InvalidLuaFilenameException*);
    void throwForLuaError(lua_State *L, const char *formatString)
        throw (LuaException*);
};

#endif
