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

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <platformstl/filesystem/filesystem_traits.hpp>
#include <platformstl/filesystem/readdir_sequence.hpp>
#include "bbconst.h"
#include "filemanager.h"
#include "bbengine.h"
#include "bblua.h"
#include "zipper.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

FileManager::FileManager(Zipper *zipper) {
  zipper_ = zipper;
  packagingListener_ = 0;
}

FileManager::~FileManager() {

}

void FileManager::setListener(PackagingListener *packagingListener) {
  packagingListener_ = packagingListener;
}

char* FileManager::loadUserLuaFilename(
    char *userDirPath, const char *metaFilename) throw (FileNotFoundException*) {
  int userPropertiesPathLen =
      (int) (strlen(userDirPath) + 1 + strlen(metaFilename));
  char *userPropertiesPath = new char[userPropertiesPathLen + 1];
  sprintf(userPropertiesPath, "%s%s%s", userDirPath, BB_DIRSEP, metaFilename);
  
  FILE *userPropertiesFile = fopen(userPropertiesPath, "r");
  if (userPropertiesFile == 0) {
    FileNotFoundException *e = new FileNotFoundException(userPropertiesPath);
    delete userPropertiesPath;
    throw e;
  }
  delete userPropertiesPath;
  
  char *userLuaFilename = new char[MAX_FILENAME_LENGTH + 1];
  fgets(userLuaFilename, MAX_FILENAME_LENGTH, userPropertiesFile);
  int userLuaLen = (int) strlen(userLuaFilename);
  fclose(userPropertiesFile);
  while (userLuaLen > 0) {
    char last = userLuaFilename[userLuaLen - 1];
    if (last == '\n' || last == '\r' || last == ' ' || last == '\t') {
      userLuaFilename[--userLuaLen] = '\0';
    } else {
      break;
    }
  }
  return userLuaFilename;
}

void FileManager::sliceString(char *filename, long start, long rest) {
  int filenameLen = (int) strlen(filename);
  for (long x = start; x < filenameLen; x++) {
    filename[x] = filename[x + rest - start];
  }
  filename[filenameLen - (rest - start)] = '\0';
}

char* FileManager::getFilePath(const char *dir, const char *filename) {
  char *absFilename;
#ifdef __WIN32__
  bool pathFromRoot = (filename[0] == BB_DIRSEP_CHR)
      || (strlen(filename) >= 3 && strncmp(&(filename[1]), ":\\", 2) == 0);
#else
  bool pathFromRoot = (filename[0] == BB_DIRSEP_CHR);
#endif
  if (pathFromRoot || dir == 0) {
    absFilename = new char[strlen(filename) + 1];
    strcpy(absFilename, filename);
    return absFilename;
  }

  long absFilenameLen = strlen(dir) + strlen(BB_DIRSEP) + strlen(filename);
  absFilename = new char[absFilenameLen + 1];
  sprintf(absFilename, "%s%s%s", dir, BB_DIRSEP, filename);
  char *dots;
  int i = 0;
  char *dotSlash = new char[3];
  sprintf(dotSlash, "%s%s", ".", BB_DIRSEP);
  while ((dots = strstr(&(absFilename[i]), dotSlash)) != NULL) {
    int offset = (int) (dots - absFilename);
    if (offset == 0 || absFilename[offset - 1] == BB_DIRSEP_CHR) {
      sliceString(absFilename, offset, offset + 2);
      i = offset;
    } else {
      i = offset + 2;
    }
  }
  delete dotSlash;

  int filenameLen = (int) strlen(absFilename);
  char *slashDot = new char[3];
  sprintf(slashDot, "%s%s", BB_DIRSEP, ".");
  if (filenameLen >= 2
      && strcmp(&(absFilename[filenameLen - 2]), slashDot) == 0) {
    absFilename[filenameLen - 2] = '\0';
  }
  delete slashDot;

  char *slashDotDot = new char[4];
  sprintf(slashDotDot, "%s%s", BB_DIRSEP, "..");
  while ((dots = strstr(absFilename, slashDotDot)) != 0) {
    long prevSlash = -1;
    for (long x = dots - absFilename - 1; x > 0; x--) {
      if (absFilename[x] == BB_DIRSEP_CHR) {
        prevSlash = x;
        break;
      }
    }
    if (prevSlash == -1) {
      return absFilename;
    }
    sliceString(absFilename, prevSlash, dots - absFilename + 3);
  }
  delete slashDotDot;

  return absFilename;
}

char* FileManager::getAbsFilePath(const char *filename) {
  char cwd[4096];
  getcwd(cwd, 4096);
  return getFilePath(cwd, filename);
}

// StageBuilder.addStageShip() takes paths relative to the main stage file, but
// its working directory should be the same as the stage's.
// Example:
//   * stages/my/game.lua calls addStageShip("gameship.lua")
//   * BerryBots loads file from stages/my/gameship.lua
//   * Ship's working directory is stages/, filename is my/gameship.lua.
char* FileManager::getStageShipRelativePath(const char *stageDir,
    const char *stageFilename, const char *stageShipName) {
  char *stagePath = FileManager::getFilePath(stageDir, stageFilename);
  char *stageLocalDir = FileManager::parseDir(stagePath);
  char *stageShipPath = FileManager::getFilePath(stageLocalDir, stageShipName);
  char *relativePath =
      FileManager::parseRelativeFilePath(stageDir, stageShipPath);

  delete stagePath;
  delete stageLocalDir;
  delete stageShipPath;
  return relativePath;
}

char* FileManager::stripExtension(const char *filename) {
  const char *extension = strrchr(filename, '.');
  int nameLength = std::min(MAX_NAME_LENGTH, (int) ((extension == 0)
      ? strlen(filename) : extension - filename));
  char *filenameRoot = new char[MAX_NAME_LENGTH];
  strncpy(filenameRoot, filename, nameLength);
  filenameRoot[nameLength] = '\0';
  return filenameRoot;
}

// srcFilename may either be a packaged ship/stage or a Lua source file. If it
// is a packaged ship/stage, it will be extracted and userDir/userFilename will
// point to the ship/stage file in the cache. Otherwise they will point to the
// dir and filename of the source file.
//
// Caller takes ownership of *userDir and *userFilename memory.
void FileManager::loadUserFileData(const char *srcBaseDir,
    const char *srcFilename, char **userDir, char **userFilename,
    const char *metaFilename, const char *cacheDir)
    throw (FileNotFoundException*, ZipperException*) {
  char *filePath = getFilePath(srcBaseDir, srcFilename);
  if (!fileExists(filePath)) {
    delete filePath;
    throw new FileNotFoundException(filePath);
  }

  int srcFilenameLen = (int) strlen(srcFilename);
  int zipLen = strlen(ZIP_EXTENSION);
  if (srcFilenameLen > zipLen
      && strcmp(&(srcFilename[srcFilenameLen - zipLen]), ZIP_EXTENSION) == 0) {
    createDirectoryIfNecessary(cacheDir);
    char *cacheSubDir = parseFilename(srcFilename);
    char *cacheDirAndSubDir = getFilePath(cacheDir, cacheSubDir);
    delete cacheSubDir;
    char *absCacheDir = getAbsFilePath(cacheDirAndSubDir);
    delete cacheDirAndSubDir;
    *userDir = absCacheDir;
    
    if (!fileExists(*userDir)) {
      createDirectory(*userDir);
      zipper_->unpackFile(filePath, *userDir);
    }

    *userFilename = loadUserLuaFilename(*userDir, metaFilename);
  } else {
    *userDir = getAbsFilePath(srcBaseDir);
    *userFilename = new char[strlen(srcFilename) + 1];
    strcpy(*userFilename, srcFilename);
  }
}

// Loads the stage from srcFilename in root directory stageBaseDir, extracts it
// to the cache if it is a packaged stage, and updates stageDir/stageFilename
// with the correct path to load the stage's .lua file. This will either be the
// same as the input or the location in the cache that we extracted to.
void FileManager::loadStageFileData(const char *stageBaseDir,
    const char *srcFilename, char **stageDir, char **stageFilename,
    const char *cacheDir) throw (FileNotFoundException*, ZipperException*) {
  loadUserFileData(stageBaseDir, srcFilename, stageDir, stageFilename,
                   STAGE_METAFILE, cacheDir);
}

// Loads the bot from srcFilename in root directory botsBaseDir, extracts it
// to the cache if it is a packaged ship/team, and updates botDir/botFilename
// with the correct path to load the stage's .lua file. This will either be the
// same as the input or the location in the cache that we extracted to.
void FileManager::loadBotFileData(const char *botsBaseDir,
    const char *srcFilename, char **botDir, char **botFilename,
    const char *cacheDir) throw (FileNotFoundException*, ZipperException*) {
  loadUserFileData(botsBaseDir, srcFilename, botDir, botFilename, BOT_METAFILE, cacheDir);
}

bool FileManager::hasExtension(const char *filename, const char *extension) {
  return ((strlen(filename) > strlen(extension))
          && (strncmp(&(filename[strlen(filename) - strlen(extension)]),
                      extension, strlen(extension)) == 0));
}

bool FileManager::isLuaFilename(const char *filename) {
  return hasExtension(filename, LUA_EXTENSION);
}

bool FileManager::isZipFilename(const char *filename) {
  return hasExtension(filename, ZIP_EXTENSION);
}

// Package files loaded by stage or bot. Expects the list of files loaded by
// the Lua state to be at the top of the stack (from "__FILES" table).
//
// Note that this list is populated by the modified Lua used by BerryBots, but
// the __FILES global could be modified by a user program, so it can't be
// completely trusted. But since a user would be packing his own code, it's not
// a big concern that he might try to hack himself. Still, only pack files with
// .lua extension, and show all files that get packaged so the user might catch
// anything fishy if they are indeed packaging someone else's code.
void FileManager::packageCommon(lua_State *userState, const char *userAbsBaseDir,
    const char *userFilename, const char *version, const char *metaFilename,
    int prevFiles, int numFiles, char **packFilenames, const char *tmpDir,
    bool obfuscate, bool force)
    throw (InvalidLuaFilenameException*, LuaException*, ZipperException*,
           FileExistsException*) {

  // For stages, prevFiles stage ships are already loaded into packFilenames.
  // Load the rest from the Lua __FILES table, on the stack.
  int x = prevFiles;
  lua_pushnil(userState);
  while (lua_next(userState, -2) != 0) {
    const char *loadedFilename = lua_tostring(userState, -1);
    checkLuaFilename(loadedFilename);
    int lenFilename = (int) strlen(loadedFilename);
    packFilenames[x] = new char[lenFilename + 1];
    strcpy(packFilenames[x], loadedFilename);
    x++;
    lua_pop(userState, 1);
  }

  // Transform bot's relative file path to packaged filename.
  // E.g.: voidious/zero.lua v1.0 => voidious.zero_1.0.tar.gz
  int baseLen = (int) (strlen(userFilename) - strlen(LUA_EXTENSION));
  int outputFilenameLen =
      (int) (baseLen + 1 + strlen(version) + strlen(ZIP_EXTENSION));
  char *outputFilename = new char[outputFilenameLen + 1];
  strncpy(outputFilename, userFilename, baseLen);
  sprintf(&(outputFilename[baseLen]), "_%s%s", version, ZIP_EXTENSION);
  for (int x = 0; x < outputFilenameLen; x++) {
    if (outputFilename[x] == BB_DIRSEP_CHR) {
      outputFilename[x] = '.';
    }
  }

  char *filesDir;
      
  if (obfuscate) {
    filesDir = new char[strlen(tmpDir) + 1];
    strcpy(filesDir, tmpDir);
  } else {
    filesDir = new char[strlen(userAbsBaseDir) + 1];
    strcpy(filesDir, userAbsBaseDir);
  }
  char *destFilename = getFilePath(userAbsBaseDir, outputFilename);

  if (!force && fileExists(destFilename)) {
    FileExistsException *e = new FileExistsException(outputFilename);
    delete outputFilename;
    delete filesDir;
    delete destFilename;
    throw e;
  }

  char *absMetaFilename;
  createDirectoryIfNecessary(tmpDir);
  char *absTmpDir = getAbsFilePath(tmpDir);
  absMetaFilename = getFilePath(absTmpDir, metaFilename);
  delete absTmpDir;
  std::ofstream fout(absMetaFilename);
  fout << userFilename;
  fout.flush();
  fout.close();
  
  if (obfuscate) {
    for (int x = 0; x < numFiles; x++) {
      if (packFilenames[x] != 0) {
        int outputFilenameLen = (int)
            (strlen(tmpDir) + strlen(BB_DIRSEP) + strlen(packFilenames[x]));
        char *outputFilename = new char[outputFilenameLen + 1];
        sprintf(outputFilename, "%s%s%s", tmpDir, BB_DIRSEP, packFilenames[x]);
        
        char *outputDir = parseDir(outputFilename);
        createDirectoryIfNecessary(outputDir);
        try {
          // TODO: Obfuscate Lua source code and save to outputFilename
//          saveBytecode(packFilenames[x], outputFilename, userAbsBaseDir);
        } catch (LuaException *e) {
          delete outputFilename;
          throw e;
        }
        
        delete outputFilename;
      }
    }
  }
  
  delete outputFilename;

  int numInputFiles = numFiles;
  for (int x = 0; x < numFiles; x++) {
    if (packFilenames[x] == 0) {
      numInputFiles--;
    }
  }
  char **inputFiles = new char*[numInputFiles];
  int z = 0;
  for (int x = 0; x < numFiles; x++) {
    if (packFilenames[x] != 0) {
      inputFiles[z++] = packFilenames[x];
    }
  }
  try {
    zipper_->packageFiles(destFilename, filesDir, inputFiles, numInputFiles,
                          obfuscate, absMetaFilename, metaFilename);
  } catch (ZipperException *e) {
    delete filesDir;
    delete destFilename;
    delete inputFiles;
    delete absMetaFilename;
    throw e;
  }

  if (packagingListener_ != 0) {
    packagingListener_->packagingComplete(packFilenames, numFiles, obfuscate,
                                          destFilename);
  }
  
  if (obfuscate) {
    // TODO: rm <tmpDir>
  }

  delete filesDir;
  delete destFilename;
  delete inputFiles;
  delete absMetaFilename;
}

void FileManager::crawlFiles(lua_State *L, const char *startFile,
    BerryBotsEngine *engine)
    throw (InvalidLuaFilenameException*, LuaException*) {
  if (luaL_loadfile(L, startFile)
      || engine->callUserLuaCode(L, 0, "", PCALL_VALIDATE)) {
    throwForLuaError(L, "Failed to load file for crawling: %s ");
  }
  
  lua_getfield(L, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) lua_objlen(L, -1);
  int numFiles2 = 0;
  do {
    numFiles2 = numFiles;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      const char *loadedFilename = lua_tostring(L, -1);
      checkLuaFilename(loadedFilename);
      if (luaL_loadfile(L, loadedFilename)
          || engine->callUserLuaCode(L, 0, "", PCALL_VALIDATE)) {
        throwForLuaError(L, "Failed to load file for crawling: %s ");
      }
      lua_pop(L, 1);
    }
    numFiles = (int) lua_objlen(L, -1);
  } while (numFiles != numFiles2);
  lua_pop(L, 1);
}

void FileManager::packageStage(const char *stageBaseDir, const char *stageName,
    const char *version, const char *cacheDir, const char *tmpDir,
    bool obfuscate, bool force)
    throw (FileNotFoundException*, InvalidLuaFilenameException*,
           LuaException*, ZipperException*, FileExistsException*) {
  checkLuaFilename(stageName);
  char *stageAbsBaseDir = getAbsFilePath(stageBaseDir);
  lua_State *stageState;
  initStageState(&stageState, stageAbsBaseDir);
  
  BerryBotsEngine engine(this);
  Stage *stage = engine.getStage();
  if (luaL_loadfile(stageState, stageName)
      || engine.callUserLuaCode(stageState, 0, "", PCALL_VALIDATE)) {
    delete stageAbsBaseDir;
    throwForLuaError(stageState, "Failed to load file for crawling: %s");
  }
  lua_getglobal(stageState, "configure");
  StageBuilder *stageBuilder = pushStageBuilder(stageState);
  stageBuilder->engine = &engine;
  if (engine.callUserLuaCode(stageState, 1, "", PCALL_VALIDATE)) {
    delete stageAbsBaseDir;
    throwForLuaError(stageState,
                     "Error calling stage function: 'configure': %s");
  }

  try {
    crawlFiles(stageState, stageName, &engine);
  } catch (InvalidLuaFilenameException *e) {
    lua_close(stageState);
    delete stageAbsBaseDir;
    throw e;
  } catch (LuaException *e) {
    delete stageAbsBaseDir;
    lua_close(stageState);
    throw e;
  }

  int numStageShips = stage->getStageShipCount();
  char **stageShipFilenames = stage->getStageShips();
  lua_getfield(stageState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) (numStageShips + lua_objlen(stageState, -1));
  char **packFilenames = new char*[numFiles];
  for (int x = 0; x < numStageShips; x++) {
    const char *stageShipFilename = stageShipFilenames[x];
    char *shipRelativePath = FileManager::getStageShipRelativePath(
        stageBaseDir, stageName, stageShipFilename);
    bool dup = false;
    for (int y = 0; y < x && !dup; y++) {
      if (strcmp(packFilenames[y], shipRelativePath) == 0) {
        packFilenames[x] = 0;
        dup = true;
      }
    }
    if (!dup) {
      int lenFilename = (int) strlen(shipRelativePath);
      packFilenames[x] = new char[lenFilename + 1];
      strcpy(packFilenames[x], shipRelativePath);
    }
  }

  try {
    packageCommon(stageState, stageAbsBaseDir, stageName, version,
        STAGE_METAFILE, numStageShips, numFiles, packFilenames, tmpDir,
        obfuscate, force);
  } catch (InvalidLuaFilenameException *e) {
    // TODO: find a way to combine these catches
    delete stageAbsBaseDir;
    lua_close(stageState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (LuaException *e) {
    delete stageAbsBaseDir;
    lua_close(stageState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (ZipperException *e) {
    delete stageAbsBaseDir;
    lua_close(stageState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (FileExistsException *e) {
    delete stageAbsBaseDir;
    lua_close(stageState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  }
  
  delete stageAbsBaseDir;
  lua_close(stageState);
  for (int x = 0; x < numFiles; x++) {
    delete packFilenames[x];
  }
  delete packFilenames;
}

// Packages the bot at botsBaseDir/botName, including all related source files,
// into a .tar.gz file in botsBaseDir.
//
// Some notes on parameters:
// botsBasedir - Base directory of the bot's source files; also the root working
//     directory for the bot's Lua state. For instance, if the bot requires the
//     "battlebot" module, it will look in "botsBaseDir/battlebot.lua".
// botName - The filename of the bot. This might include a relative path to
//     the file. For instance, I may have my main bot file in
//     "bots/voidious/zero.lua" and require the "battlebot" module in
//     "bots/battlebot.lua". In this case, botName would be "voidious/zero.lua".
void FileManager::packageBot(const char *botBaseDir, const char *botName,
    const char *version, const char *cacheDir, const char *tmpDir,
    bool obfuscate, bool force)
    throw (FileNotFoundException*, InvalidLuaFilenameException*,
           LuaException*, ZipperException*, FileExistsException*) {
  checkLuaFilename(botName);
  char *botAbsBaseDir = getAbsFilePath(botBaseDir);
  lua_State *shipState;
  initShipState(&shipState, botAbsBaseDir);
  BerryBotsEngine engine(this);
  crawlFiles(shipState, botName, &engine);

  lua_getfield(shipState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) lua_objlen(shipState, -1);
  char **packFilenames = new char*[numFiles];

  try {
    packageCommon(shipState, botAbsBaseDir, botName, version,
                  BOT_METAFILE, 0, numFiles, packFilenames, tmpDir, obfuscate,
                  force);
  } catch (InvalidLuaFilenameException *e) {
    // TODO: find a way to combine these catches
    delete botAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (LuaException *e) {
    delete botAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (ZipperException *e) {
    delete botAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (FileExistsException *e) {
    delete botAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  }

  delete botAbsBaseDir;
  lua_close(shipState);
  for (int x = 0; x < numFiles; x++) {
    delete packFilenames[x];
  }
  delete packFilenames;
}

bool FileManager::fileExists(const char *filename) {
  FILE *testFile = fopen(filename, "r");
  bool exists = (testFile != 0);
  if (exists) {
    fclose(testFile);
  }
  return exists;
}

bool FileManager::isDirectory(const char *filePath) {
  DIR *dir = opendir(filePath);
  if (dir == 0) {
    return false;
  } else {
    closedir(dir);
    return true;
  }
}

void FileManager::createDirectory(const char *filename) {
  platformstl::filesystem_traits<char> traits;
  traits.create_directory(filename);
}

char* FileManager::parseFilename(const char *dirAndFilename) {
  const char *fromFinalSlash = strrchr(dirAndFilename, BB_DIRSEP_CHR);
  char *filename;
  if (fromFinalSlash == 0) {
    filename = new char[strlen(dirAndFilename) + 1];
    strcpy(filename, dirAndFilename);
  } else {
    filename = new char[strlen(&(fromFinalSlash[1])) + 1];
    strcpy(filename, &(fromFinalSlash[1]));
  }
  return filename;
}

char* FileManager::parseRelativeFilePath(const char *absBaseDir,
                                         const char *absFilePath) {
  if (strstr(absFilePath, absBaseDir) != absFilePath) {
    return 0;
  }

  int absBaseDirLen = (int) strlen(absBaseDir);
  int absFilePathLen = (int) strlen(absFilePath);
  if (absBaseDirLen == absFilePathLen) {
    char *dot = new char[2];
    strcpy(dot, ".");
    return dot;
  }
  int relativePathLen = (int) absFilePathLen - absBaseDirLen;
  char *relativePath = new char[relativePathLen + 1];
  strcpy(relativePath, &(absFilePath[absBaseDirLen + 1]));
  return relativePath;
}

void FileManager::createDirectoryIfNecessary(const char *dir) {
  char *parentDir = parseDir(dir);
  if (strcmp(".", parentDir)) {
    createDirectoryIfNecessary(parentDir);
    delete parentDir;
  }
  if (!fileExists(dir)) {
    createDirectory(dir);
  }
}

char* FileManager::parseDir(const char *dirAndFilename) {
  const char *fromFinalSlash = strrchr(dirAndFilename, BB_DIRSEP_CHR);
  if (fromFinalSlash == 0) {
    char *dot = new char[2];
    strcpy(dot, ".");
    return dot;
  } else {
    long dirLen = fromFinalSlash - dirAndFilename;
    if (dirLen == 0) {
      char *dot = new char[2];
      strcpy(dot, ".");
      return dot;
    }
    char *dir = new char[dirLen + 1];
    strncpy(dir, dirAndFilename, dirLen);
    dir[dirLen] = '\0';
    return dir;
  }
}

void FileManager::checkLuaFilename(const char *filename)
    throw (InvalidLuaFilenameException*) {
  if (!isLuaFilename(filename)) {
    throw new InvalidLuaFilenameException(filename);
  }
}

// adapted from luac.c
static int writer(lua_State* L, const void* p, size_t size, void* u) {
  return (fwrite(p,size,1,(FILE*)u)!=1) && (size!=0);
}

void FileManager::saveBytecode(const char *srcFile, const char *outputFile,
                               const char *luaCwd)
    throw (LuaException*) {
  lua_State *saveState = luaL_newstate();
  lua_setcwd(saveState, luaCwd);
  if (luaL_loadfile(saveState, srcFile) != 0) {
    const char *luaMessage = lua_tostring(saveState, -1);
    const char *formatString = "Lua failed to load file: %s";
    int messageLen = (int) (strlen(formatString) + strlen(luaMessage) - 2);
    char *errorMessage = new char[messageLen + 1];
    sprintf(errorMessage, formatString, luaMessage);
    LuaException *e = new LuaException(errorMessage);
    delete errorMessage;

    throw e;
  }
  FILE* D = fopen(outputFile, "wb");
  lua_dump(saveState, writer, D);
  lua_close(saveState);
  fclose(D);
}

void FileManager::deleteFromCache(const char *cacheDir, const char *filename) {
  char *cacheFilePath = getFilePath(cacheDir, filename);
  recursiveDelete(cacheFilePath);
  delete cacheFilePath;
}

void FileManager::recursiveDelete(const char *fileToDelete) {
  if (isDirectory(fileToDelete)) {
    platformstl::readdir_sequence dir(fileToDelete,
        platformstl::readdir_sequence::files
            | platformstl::readdir_sequence::directories);
    platformstl::readdir_sequence::const_iterator first = dir.begin();
    platformstl::readdir_sequence::const_iterator last = dir.end();
    while (first != last) {
      platformstl::readdir_sequence::const_iterator file = first++;
      char *filename = (char *) *file;
      char *filePath = getFilePath(fileToDelete, filename);
      recursiveDelete(filePath);
      delete filePath;
    }
  }
  remove(fileToDelete);
}

void FileManager::throwForLuaError(lua_State *L, const char *formatString)
    throw (LuaException*) {
  const char *luaMessage = lua_tostring(L, -1);
  int messageLen = (int) (strlen(formatString) + strlen(luaMessage) - 2);
  char *errorMessage = new char[messageLen + 1];
  sprintf(errorMessage, formatString, luaMessage);
  LuaException *e = new LuaException(errorMessage);
  delete errorMessage;
  lua_close(L);
  
  throw e;
}

LuaException::LuaException(const char *details) {
  message_ = new char[strlen(details) + 41];
  sprintf(message_, "Lua processing failure: %s", details);
}

const char* LuaException::what() const throw() {
  return message_;
}

LuaException::~LuaException() throw() {
  delete message_;
}

FileNotFoundException::FileNotFoundException(const char *filename) {
  message_ = new char[strlen(filename) + 17];
  sprintf(message_, "File not found: %s", filename);
}

FileNotFoundException::~FileNotFoundException() throw() {
  delete message_;
}

const char* FileNotFoundException::what() const throw() {
  return message_;
}

InvalidLuaFilenameException::InvalidLuaFilenameException(const char *filename) {
  message_ = new char[strlen(filename) + 17];
  sprintf(message_, "Invalid Lua filename: %s (must end with .lua)", filename);
}

InvalidLuaFilenameException::~InvalidLuaFilenameException() throw() {
  delete message_;
}

const char* InvalidLuaFilenameException::what() const throw() {
  return message_;
}

FileExistsException::FileExistsException(const char *filename) {
  filename_ = new char[strlen(filename) + 1];
  strcpy(filename_, filename);
}

FileExistsException::~FileExistsException() throw() {
  delete filename_;
}

const char* FileExistsException::what() const throw() {
  return filename_;
}
