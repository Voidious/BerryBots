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
#include <sys/stat.h>
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

FileManager::FileManager() {
  zipper_ = new NullZipper();
  ownZipper_ = true;
  packagingListener_ = 0;
}

FileManager::FileManager(Zipper *zipper) {
  zipper_ = zipper;
  ownZipper_ = false;
  packagingListener_ = 0;
}

FileManager::~FileManager() {
  if (ownZipper_) {
    delete zipper_;
  }
}

void FileManager::setListener(PackagingListener *packagingListener) {
  packagingListener_ = packagingListener;
}

char* FileManager::loadUserLuaFilename(char *userDirPath,
    const char *metaFilename) throw (FileNotFoundException*) {
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
  fixSlashes(userLuaFilename);
  return userLuaFilename;
}

void FileManager::sliceString(char *filename, long start, long rest) {
  int filenameLen = (int) strlen(filename);
  for (long x = start; x < filenameLen; x++) {
    filename[x] = filename[x + rest - start];
  }
  filename[filenameLen - (rest - start)] = '\0';
}

bool FileManager::isAbsPath(const char *filename) {
#ifdef __WIN32__
  return (filename[0] == BB_DIRSEP_CHR)
      || (strlen(filename) >= 3 && strncmp(&(filename[1]), ":\\", 2) == 0);
#else
  return (filename[0] == BB_DIRSEP_CHR);
#endif
}

char* FileManager::getFilePath(const char *dir, const char *filename) {
  char *absFilename;
  if (isAbsPath(filename) || dir == 0) {
    absFilename = new char[strlen(filename) + 1];
    strcpy(absFilename, filename);
    return absFilename;
  }

  long absFilenameLen = strlen(dir) + strlen(BB_DIRSEP) + strlen(filename);
  absFilename = new char[absFilenameLen + 1];
  sprintf(absFilename, "%s%s%s", dir, BB_DIRSEP, filename);

  fixSlashes(absFilename);

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
char* FileManager::getStageShipRelativePath(const char *stagesDir,
    const char *stageFilename, const char *stageShipName) {
  char *stagePath = FileManager::getFilePath(stagesDir, stageFilename);
  char *stageLocalDir = FileManager::parseDir(stagePath);
  char *stageShipPath = FileManager::getFilePath(stageLocalDir, stageShipName);
  char *relativePath =
      FileManager::parseRelativeFilePath(stagesDir, stageShipPath);

  delete stagePath;
  delete stageLocalDir;
  delete stageShipPath;
  return relativePath;
}

char* FileManager::stripExtension(const char *filename) {
  if (isLuaFilename(filename)) {
    return stripLastExtension(filename);
  } else if (isZipFilename(filename)) {
    char *gzStripped = stripLastExtension(filename);
    char *stripped = stripLastExtension(gzStripped);
    delete gzStripped;
    int strippedLen = (int) strlen(stripped);
    for (int x = strippedLen - 1; x >= 0; x--) {
      if (stripped[x] == '_') {
        stripped[x] = ' ';
        break;
      }
    }
    return stripped;
  } else {
    char *stripped = new char[strlen(filename) + 1];
    strcpy(stripped, filename);
    return stripped;
  }
}

char* FileManager::stripLastExtension(const char *filename) {
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
    throw (FileNotFoundException*, ZipperException*,
           PackagedSymlinkException*) {
  char *filePath = getFilePath(srcBaseDir, srcFilename);
  if (!fileExists(filePath)) {
    FileNotFoundException *e = new FileNotFoundException(filePath);
    delete filePath;
    throw e;
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

    if (hasSymlinks(*userDir)) {
      std::string symlinkError("Can't load package with symlinks: ");
      symlinkError.append(srcFilename);
      throw new PackagedSymlinkException(symlinkError.c_str());
    }

    *userFilename = loadUserLuaFilename(*userDir, metaFilename);
  } else {
    *userDir = getAbsFilePath(srcBaseDir);
    *userFilename = new char[strlen(srcFilename) + 1];
    strcpy(*userFilename, srcFilename);
  }
  delete filePath;
}

// TODO: Find a way to make this fail on broken symlinks, too. For now I don't
//       think it's a security concern.
bool FileManager::hasSymlinks(const char *userDir) {
#ifdef __WIN32__
  return false;
#else
  bool foundLink = false;
  if (isDirectory(userDir)) {
    platformstl::readdir_sequence dir(userDir,
        platformstl::readdir_sequence::files
            | platformstl::readdir_sequence::directories);
    platformstl::readdir_sequence::const_iterator first = dir.begin();
    platformstl::readdir_sequence::const_iterator last = dir.end();
    while (first != last && !foundLink) {
      platformstl::readdir_sequence::const_iterator file = first++;
      char *filename = (char *) *file;
      char *filePath = getFilePath(userDir, filename);
      struct stat lst;
      lstat(filePath, &lst);
      foundLink |= S_ISLNK(lst.st_mode);
      if (isDirectory(filePath)) {
        foundLink |= hasSymlinks(filePath);
      } else {
        struct stat st;
        stat(filePath, &st);
        foundLink |= !S_ISREG(st.st_mode);
      }
      delete filePath;
    }
  }
  return foundLink;
#endif
}

// Loads the stage from srcFilename in root directory stagesBaseDir, extracts it
// to the cache if it is a packaged stage, and updates stagesDir/stageFilename
// with the correct path to load the stage's .lua file. This will either be the
// same as the input or the location in the cache that we extracted to.
void FileManager::loadStageFileData(const char *stagesBaseDir,
    const char *srcFilename, char **stagesDir, char **stageFilename,
    const char *cacheDir) throw (FileNotFoundException*, ZipperException*,
                                 PackagedSymlinkException*) {
  loadUserFileData(stagesBaseDir, srcFilename, stagesDir, stageFilename,
                   STAGE_METAFILE, cacheDir);
}

// Loads the ship from srcFilename in root directory shipsBaseDir, extracts it
// to the cache if it is a packaged ship/team, and updates shipDir/shipFilename
// with the correct path to load the stage's .lua file. This will either be the
// same as the input or the location in the cache that we extracted to.
void FileManager::loadShipFileData(const char *shipsBaseDir,
    const char *srcFilename, char **shipDir, char **shipFilename,
    const char *cacheDir) throw (FileNotFoundException*, ZipperException*,
                                 PackagedSymlinkException*) {
  loadUserFileData(shipsBaseDir, srcFilename, shipDir, shipFilename,
                   SHIP_METAFILE, cacheDir);
}

char* FileManager::getStageDescription(const char *stagesBaseDir,
    const char *srcFilename, const char *cacheDir)
    throw (FileNotFoundException*, ZipperException*,
           PackagedSymlinkException*) {
  char *stagesDir;
  char *stageFilename;
  loadStageFileData(stagesBaseDir, srcFilename, &stagesDir, &stageFilename,
                    cacheDir);
  char *stagePath = getFilePath(stagesDir, stageFilename);
  delete stagesDir;
  delete stageFilename;

  FILE *stageFile = fopen(stagePath, "r");
  if (stageFile == NULL) {
    FileNotFoundException *e = new FileNotFoundException(srcFilename);
    delete stagePath;
    throw e;
  }
  std::string description;
  char *fileLine = new char[1024];
  bool done = false;
  bool blockComment = false;
  while (!done) {
    if (fgets(fileLine, 1024, stageFile) == NULL) {
      done = true;
    } else {
      if (feof(stageFile)
          || (!blockComment && !isWhitespace(fileLine)
              && (strlen(fileLine) < 2 || strncmp(fileLine, "--", 2)))) {
        done = true;
      } else {
        if (blockComment) {
          const char *endBlock = strstr(fileLine, "]]");
          if (endBlock != 0) {
            description.append(fileLine, endBlock - fileLine);
            done = true;
          } else {
            description.append(fileLine);
          }
        } else if (strlen(fileLine) >= 4 && strncmp(fileLine, "--[[", 4) == 0) {
          blockComment = true;
        } else if (isWhitespace(fileLine)) {
          description.append(fileLine);
        } else {
          description.append(&(fileLine[2]));
        }
      }
    }
  }
  delete fileLine;
  delete stagePath;
  fclose(stageFile);

  char *descCopy = new char[description.length() + 1];
  strcpy(descCopy, description.c_str());
  return descCopy;
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

// Package files loaded by stage or ship. Expects the list of files loaded by
// the Lua state to be at the top of the stack (from "__FILES" table).
//
// Note that this list is populated by the modified Lua used by BerryBots, but
// the __FILES global could be modified by a user program, so it can't be
// completely trusted. But since a user would be packing his own code, it's not
// a big concern that he might try to hack himself. Still, only pack files with
// .lua extension, and show all files that get packaged so the user might catch
// anything fishy if they are indeed packaging someone else's code.
void FileManager::packageCommon(lua_State *userState,
    const char *userAbsBaseDir, const char *userFilename, const char *version,
    const char *metaFilename, int prevFiles, int numFiles, char **packFilenames,
    const char *tmpDir, bool obfuscate, bool force)
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

  // Transform ship's relative file path to packaged filename.
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
  long lenUserFilename = strlen(userFilename);
  char *slashedUserFilename = new char[lenUserFilename + 1];
  strcpy(slashedUserFilename, userFilename);
  for (int x = 0; x < lenUserFilename; x++) {
    if (slashedUserFilename[x] == '\\') {
      slashedUserFilename[x] = '/';
    }
  }
  std::ofstream fout(absMetaFilename);
  fout << slashedUserFilename;
  fout.flush();
  fout.close();
  delete slashedUserFilename;
  
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
          delete destFilename;
          delete absMetaFilename;
          delete outputFilename;
          delete outputDir;
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
    recursiveDelete(tmpDir);
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

void FileManager::packageStage(const char *stagesBaseDir, const char *stageName,
    const char *version, const char *cacheDir, const char *tmpDir,
    bool obfuscate, bool force)
    throw (FileNotFoundException*, InvalidLuaFilenameException*,
           LuaException*, ZipperException*, FileExistsException*,
           InvalidStageShipException*) {
  checkLuaFilename(stageName);
  char *stageAbsBaseDir = getAbsFilePath(stagesBaseDir);
  lua_State *stageState;
  initStageState(&stageState, stageAbsBaseDir);
  
  BerryBotsEngine engine(0, this, 0);
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
        stagesBaseDir, stageName, stageShipFilename);
    if (shipRelativePath == 0) {
      for (int y = 0; y < x; y++) {
        if (packFilenames[y] != 0) {
          delete packFilenames[y];
        }
      }
      delete packFilenames;
      delete stageAbsBaseDir;
      throw new InvalidStageShipException(stageShipFilename);
    }
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
    delete shipRelativePath;
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

// Packages the ship at shipsBaseDir/shipName, including all related source
// files, into a .tar.gz file in shipsBaseDir.
//
// Some notes on parameters:
// shipsBaseDir - Base directory of the ship's source files; also the root
//     working directory for the ship's Lua state. For instance, if the ship
//     requires the "battlebot" module, it will look in
//     "shipsBaseDir/battlebot.lua".
// shipName - The filename of the ship. This might include a relative path to
//     the file. For instance, I may have my main ship file in
//     "bots/voidious/zero.lua" and require the "battlebot" module in
//     "bots/battlebot.lua", and shipName would be "voidious/zero.lua".
void FileManager::packageShip(const char *shipBaseDir, const char *shipName,
    const char *version, const char *cacheDir, const char *tmpDir,
    bool obfuscate, bool force)
    throw (FileNotFoundException*, InvalidLuaFilenameException*,
           LuaException*, ZipperException*, FileExistsException*) {
  checkLuaFilename(shipName);
  char *shipAbsBaseDir = getAbsFilePath(shipBaseDir);
  lua_State *shipState;
  initShipState(&shipState, shipAbsBaseDir);
  BerryBotsEngine engine(0, this, 0);
  crawlFiles(shipState, shipName, &engine);

  lua_getfield(shipState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) lua_objlen(shipState, -1);
  char **packFilenames = new char*[numFiles];

  try {
    packageCommon(shipState, shipAbsBaseDir, shipName, version, SHIP_METAFILE, 0,
                  numFiles, packFilenames, tmpDir, obfuscate, force);
  } catch (InvalidLuaFilenameException *e) {
    // TODO: find a way to combine these catches
    delete shipAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (LuaException *e) {
    delete shipAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (ZipperException *e) {
    delete shipAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  } catch (FileExistsException *e) {
    delete shipAbsBaseDir;
    lua_close(shipState);
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;
    throw e;
  }

  delete shipAbsBaseDir;
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
  return exists || isDirectory(filename);
}

void FileManager::fixSlashes(char *filename) {
  int filenameLen = (int) strlen(filename);
  for (int x = 0; x < filenameLen; x++) {
    if (filename[x] == '/' || filename[x] == '\\') {
      filename[x] = BB_DIRSEP_CHR;
    }
  }

  char *slashes;
  int i = 0;
  char *slashSlash = new char[3];
  sprintf(slashSlash, "%s%s", BB_DIRSEP, BB_DIRSEP);
  while ((slashes = strstr(&(filename[i]), slashSlash)) != NULL) {
    int offset = (int) (slashes - filename);
    sliceString(filename, offset, offset + 1);
    i = offset;
  }
  delete slashSlash;
}

char* FileManager::readFile(const char *filename)
    throw (FileNotFoundException*) {
  FILE *f = fopen(filename, "r");
  if (f == 0) {
    throw new FileNotFoundException(filename);
  }

  std::string contentsString;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
    contentsString.append(line);
  }
  fclose(f);
  char *contents = new char[contentsString.size() + 1];
  strcpy(contents, contentsString.c_str());
  return contents;
}

void FileManager::writeFile(const char *filename, const char *contents) {
  char *dir = parseDir(filename);
  createDirectoryIfNecessary(dir);
  delete dir;

  FILE *f = fopen(filename, "w");
  fputs(contents, f);
  fclose(f);
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
  }
  if (!fileExists(dir)) {
    createDirectory(dir);
  }
  delete parentDir;
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
  std::string errorMessage("Invalid Lua filename: ");
  errorMessage.append(filename);
  errorMessage.append(" (must end with .lua)");
  message_ = new char[errorMessage.size() + 1];
  strcpy(message_, errorMessage.c_str());
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

PackagedSymlinkException::PackagedSymlinkException(const char *details) {
  message_ = new char[strlen(details) + 41];
  sprintf(message_, "Lua processing failure: %s", details);
}

const char* PackagedSymlinkException::what() const throw() {
  return message_;
}

PackagedSymlinkException::~PackagedSymlinkException() throw() {
  delete message_;
}

InvalidStageShipException::InvalidStageShipException(const char *filename) {
  std::string errorMessage("Stage ship outside of stages directory: ");
  errorMessage.append(filename);
  message_ = new char[errorMessage.size() + 1];
  strcpy(message_, errorMessage.c_str());
}

InvalidStageShipException::~InvalidStageShipException() throw() {
  delete message_;
}

const char* InvalidStageShipException::what() const throw() {
  return message_;
}
