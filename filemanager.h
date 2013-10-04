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
#include "zipper.h"

#define MAX_LINE_LENGTH  16384

extern "C" {
  #include "lua.h"
}

class BerryBotsEngine;

class PackagingListener {
  public:
    virtual void packagingComplete(char **sourceFiles, int numFiles,
        bool obfuscate, const char *destinationFile) = 0;
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

class PackagedSymlinkException : public std::exception {
  char *message_;
  public:
    PackagedSymlinkException(const char *details);
    ~PackagedSymlinkException() throw();
    virtual const char* what() const throw();
};

class InvalidStageShipException : public std::exception {
  char *message_;
  public:
    InvalidStageShipException(const char *filename);
    ~InvalidStageShipException() throw();
    virtual const char* what() const throw();
};

class NullZipper : public Zipper {
  public:
    NullZipper() {};
    virtual void packageFiles(const char *outputFile, const char *baseDir,
        char **filenames, int numFiles, bool binary,
        const char *absMetaFilename, const char *metaFilename)
        throw (ZipperException*) {};
    virtual void unpackFile(const char *zipFile, const char *outputDir)
        throw (ZipperException*) {};
};

class FileManager {
  Zipper *zipper_;
  PackagingListener *packagingListener_;
  bool ownZipper_;

  public:
    FileManager();
    FileManager(Zipper *zipper);
    ~FileManager();
    void setListener(PackagingListener *packagingListener);
    void loadStageFileData(const char *stagesBaseDir, const char *srcFilename,
        char **stagesDir, char **stageFilename, const char *cacheDir)
        throw (FileNotFoundException*, ZipperException*,
               PackagedSymlinkException*);
    void loadShipFileData(const char *shipsBaseDir, const char *srcFilename,
        char **shipDir, char **shipFilename, const char *cacheDir)
        throw (FileNotFoundException*, ZipperException*,
               PackagedSymlinkException*);
    char* getStageDescription(const char *stagesBaseDir, const char *srcFilename,
        const char *cacheDir) throw (FileNotFoundException*, ZipperException*,
                                     PackagedSymlinkException*);
    bool isLuaFilename(const char *filename);
    bool isZipFilename(const char *filename);
    void packageStage(const char *stagesBaseDir, const char *stageName,
                      const char *version, const char *cacheDir,
                      const char *tmpDir, bool obfuscate, bool force)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*, ZipperException*, FileExistsException*,
               InvalidStageShipException*);
    void packageShip(const char *shipBaseDir, const char *shipName,
                     const char *version, const char *cacheDir,
                     const char *tmpDir, bool obfuscate, bool force)
        throw (FileNotFoundException*, InvalidLuaFilenameException*,
               LuaException*, ZipperException*, FileExistsException*);
    void saveBytecode(const char *srcFile, const char *outputFile,
                      const char *luaCwd)
        throw (LuaException*);
    void deleteFromCache(const char *cacheDir, const char *filename);
    bool isAbsPath(const char *filename);
    char* getFilePath(const char *dir, const char *filename);
    char* getAbsFilePath(const char *filename);
    char* parseDir(const char *dirAndFilename);
    char* parseFilename(const char *dirAndFilename);
    char* parseRelativeFilePath(const char *absBaseDir,
                                const char *absFilePath);
    char* getStageShipRelativePath(const char *stagesDir,
        const char *stageFilename, const char *stageShipName);
    char* stripExtension(const char *filename);
    bool isDirectory(const char *filePath);
    bool fileExists(const char *filename);
    void fixSlashes(char *filename);
    char* readFile(const char *filename) throw (FileNotFoundException*);
    void writeFile(const char *filename, const char *contents);
    void createDirectory(const char *filename);
    void createDirectoryIfNecessary(const char *dir);
    void recursiveDelete(const char *fileToDelete);
  private:
    char* loadUserLuaFilename(char *userDirPath, const char *metaFilename)
        throw (FileNotFoundException*);
    void sliceString(char *filename, long start, long rest);
    char* stripLastExtension(const char *filename);
    void loadUserFileData(const char *srcBaseDir, const char *srcFilename,
        char **userDir, char **userFilename, const char *metaFilename,
        const char *cacheDir) throw (FileNotFoundException*, ZipperException*,
                                     PackagedSymlinkException*);
    bool hasSymlinks(const char *userDir);
    bool hasExtension(const char *filename, const char *extension);
    void packageCommon(lua_State *userState, const char *userAbsBaseDir,
        const char *userFilename, const char *version, const char *metaFilename,
        int prevFiles, int numFiles, char **packFilenames, const char *tmpDir,
        bool obfuscate, bool force)
        throw (InvalidLuaFilenameException*, LuaException*, ZipperException*,
               FileExistsException*);
    void crawlFiles(lua_State *L, const char *startFile,
                    BerryBotsEngine *engine)
                    throw (InvalidLuaFilenameException*, LuaException*);
    void checkLuaFilename(const char *filename)
        throw (InvalidLuaFilenameException*);
    void throwForLuaError(lua_State *L, const char *formatString)
        throw (LuaException*);
};

#endif
