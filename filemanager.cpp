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
#include <iostream>
using namespace std;
#include <unistd.h>
#include "bbconst.h"
#include "filemanager.h"
#include "bbengine.h"
#include "bblua.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

// TODO: refactor this to ditch the globals
extern BerryBotsEngine *engine;
extern Stage *stage;

FileManager::FileManager() {
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

char* FileManager::getAbsoluteFilename(char *dir, char *filename) {
  char *absFilename;
  // TODO: Can "C:/" also be absolute path on Windows?
  if (dir == 0 || filename[0] == BB_DIRSEP_CHR
      || (strlen(filename) > 1 && strncmp(&(filename[1]), ":\\", 2) == 0)) {
    absFilename = new char[strlen(filename) + 1];
    strcpy(absFilename, filename);
    return absFilename;
  }
  
  absFilename = new char[strlen(dir) + strlen(filename) + 2];
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

void FileManager::loadUserFile(const char *srcFilename, char **userDir,
    char **userFilename, char **userCwd, const char *metaFilename,
    const char *cacheDir) throw (FileNotFoundException*) {
  int srcFilenameLen = (int) strlen(srcFilename);
  int zipLen = strlen(ZIP_EXTENSION);
  if (!fileExists(srcFilename)) {
    throw new FileNotFoundException(srcFilename);
  }
  if (srcFilenameLen > zipLen
      && strcmp(&(srcFilename[srcFilenameLen - zipLen]), ZIP_EXTENSION) == 0) {
    createDirIfNecessary(cacheDir);
    
    const char *userDirName = parseFilename(srcFilename);
    int userDirPathLen = (int) (strlen(cacheDir) + 1 + strlen(userDirName));
    
    char *userDirPath = new char[userDirPathLen + 1];
    sprintf(userDirPath, "%s%s%s", cacheDir, BB_DIRSEP, userDirName);
    *userDir = userDirPath;
    
    if (!fileExists(userDirPath)) {
      mkdir(userDirPath);
      int extractCmdLen = 20 + srcFilenameLen + 4 + userDirPathLen;
      char *extractCmd = new char[extractCmdLen + 1];
      sprintf(extractCmd, "tar xfv %s -C %s > /dev/null", srcFilename,
              userDirPath);
      system(extractCmd);
      delete extractCmd;
    }
    
    *userFilename = loadUserLuaFilename(userDirPath, metaFilename);
  } else {
    *userDir = parseDir(srcFilename);
    *userFilename = parseFilename(srcFilename);
  }
  
  char cwd[1024];
  getcwd(cwd, 1024);
  if (*userDir == 0) {
    *userCwd = new char[strlen(cwd) + 1];
    strcpy(*userCwd, cwd);
  } else {
    if ((*userDir)[0] == BB_DIRSEP_CHR) {
      *userCwd = new char[strlen(*userDir) + 1];
      strcpy(*userCwd, *userDir);
    } else {
      *userCwd = getAbsoluteFilename(cwd, *userDir);
    }
  }
}

void FileManager::loadStageFile(const char *srcFilename, char **stageDir,
    char **stageFilename, char **stageCwd, const char *cacheDir)
    throw (FileNotFoundException*) {
  loadUserFile(
      srcFilename, stageDir, stageFilename, stageCwd, STAGE_METAFILE, cacheDir);
}

void FileManager::loadBotFile(const char *srcFilename, char **botDir,
    char **botFilename, char **botCwd, const char *cacheDir)
    throw (FileNotFoundException*) {
  loadUserFile(
      srcFilename, botDir, botFilename, botCwd, BOT_METAFILE, cacheDir);
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
// TODO: Make this non-hideous.
void FileManager::packageCommon(lua_State *userState, char *userDir,
    char *userFilename, char *luaCwd, const char *version,
    const char *metaFilename, int prevFiles, int numFiles, int prevCmdLen,
    char **packFilenames, const char *tmpDir, bool nosrc)
    throw (InvalidLuaFilenameException*) {
  int x = prevFiles;
  lua_pushnil(userState);
  int cmdLen = prevCmdLen;
  while (lua_next(userState, -2) != 0) {
    const char *loadedFilename = lua_tostring(userState, -1);
    checkLuaFilename(loadedFilename);
    int lenFilename = (int) strlen(loadedFilename);
    cmdLen += 3 + lenFilename;
    packFilenames[x] = new char[lenFilename + 1];
    strcpy(packFilenames[x], loadedFilename);
    x++;
    lua_pop(userState, 1);
  }
  lua_pop(userState, 1);
  
  if (nosrc) {
    createDirIfNecessary(tmpDir);
    for (int x = 0; x < numFiles; x++) {
      if (packFilenames[x] != 0) {
        char *outputFilename =
        new char[strlen(tmpDir) + 1 + strlen(packFilenames[x]) + 1];
        sprintf(outputFilename, "%s%s%s", tmpDir, BB_DIRSEP, packFilenames[x]);
        
        char *outputDir = parseDir(outputFilename);
        if (outputDir != 0) {
          mkdirIfNecessary(outputDir);
        }
        saveBytecode(packFilenames[x], outputFilename, userDir);
        
        delete outputFilename;
      }
    }
  }
  if (nosrc) {
    // 'cd "<tmpDir>"; '
    cmdLen += 4 + strlen(tmpDir) + 3;
    // ../
    cmdLen += 3;
    if (userDir != 0) {
      // <userDir>/
      cmdLen += strlen(userDir) + 1;
    }
  } else if (userDir != 0) {
    // 'cd "<userDir>"; '
    cmdLen += 4 + strlen(userDir) + 3;
  }
  // 'echo "<userFilename>" > <metaFilename>; '
  cmdLen += 6 + strlen(userFilename) + 4 + strlen(metaFilename) + 2;
  // 'tar czfv "<userFilename - .lua + _version>.tar.gz" "<file1>" "<file2>" ... <metaFilename> > /dev/null; '
  cmdLen += 10 + strlen(userFilename) - 4 + 1 + strlen(version) + 8 + 1
  + strlen(metaFilename) + 14;
  // "rm <metaFilename>\0"
  cmdLen += 3 + strlen(metaFilename) + 1;
  
  int baseLen = (int) (strlen(userFilename) - strlen(LUA_EXTENSION));
  int outputFilenameLen =
      (int) (baseLen + 1 + strlen(version) + strlen(ZIP_EXTENSION));
  char *outputFilename = new char[outputFilenameLen + 1];
  strncpy(outputFilename, userFilename, baseLen);
  sprintf(&(outputFilename[baseLen]), "_%s%s", version, ZIP_EXTENSION);
  
  char *destFilename;
  if (nosrc) {
    // ../<userDir>/outputFilename
    destFilename = new char[3 + (userDir == 0 ? 0 : strlen(userDir) + 1)
                            + outputFilenameLen + 1];
    if (userDir == 0) {
      sprintf(destFilename, "../%s", outputFilename);
    } else {
      sprintf(destFilename, "../%s/%s", userDir, outputFilename);
    }
  } else {
    destFilename = outputFilename;
  }
  
  char *packCmd = new char[cmdLen];
  packCmd[0] = '\0';
  if (nosrc) {
    sprintf(packCmd, "cd \"%s\"; ", tmpDir);
  } else if (userDir != 0) {
    sprintf(packCmd, "cd \"%s\"; ", userDir);
  }
  sprintf(&(packCmd[strlen(packCmd)]), "echo \"%s\" > %s; tar czfv \"%s\"",
          userFilename, metaFilename, destFilename);
  
  for (int x = 0; x < numFiles; x++) {
    if (packFilenames[x] != 0) {
      strcat(packCmd, " \"");
      strcat(packCmd, packFilenames[x]);
      strcat(packCmd, "\"");
    }
  }
  sprintf(&(packCmd[strlen(packCmd)]), " %s > /dev/null; rm %s", metaFilename,
          metaFilename);
  
  lua_close(userState);
  system(packCmd);
  
  if (packagingListener_ != 0) {
    packagingListener_->packagingComplete(packFilenames, numFiles,
                                          outputFilename);
  }
  
  if (nosrc) {
    delete destFilename;
    
    // rm <tmpDir>
    char *deleteCmd = new char[7 + strlen(tmpDir) + 1];
    strcpy(deleteCmd, "rm -rf ");
    strcat(deleteCmd, tmpDir);
    system(deleteCmd);
    delete deleteCmd;
  }
  
  delete outputFilename;
  delete packCmd;
}

void FileManager::crawlFiles(lua_State *L, const char *startFile)
    throw (InvalidLuaFilenameException*) {
  if (luaL_loadfile(L, startFile)
      || lua_pcall(L, 0, 0, 0)) {
    luaL_error(L, "failed to crawl file: %s", lua_tostring(L, -1));
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
      if (luaL_loadfile(L, loadedFilename) || lua_pcall(L, 0, 0, 0)) {
        luaL_error(L, "failed to crawl file: %s", lua_tostring(L, -1));
      }
      lua_pop(L, 1);
    }
    numFiles = (int) lua_objlen(L, -1);
  } while (numFiles != numFiles2);
  lua_pop(L, 1);
}

void FileManager::packageStage(const char *stageArg, const char *version,
    const char *cacheDir, const char *tmpDir, bool nosrc)
    throw (FileNotFoundException*, InvalidLuaFilenameException*) {
  lua_State *stageState;
  char *stageDir;
  char *stageFilename;
  char *stageCwd;
  loadStageFile(stageArg, &stageDir, &stageFilename, &stageCwd, cacheDir);
  checkLuaFilename(stageFilename);
  initStageState(&stageState, stageCwd, stageFilename);
  
  engine = new BerryBotsEngine();
  stage = engine->getStage();
  if (luaL_loadfile(stageState, stageFilename)
      || lua_pcall(stageState, 0, 0, 0)) {
    luaL_error(stageState, "cannot load stage file: %s",
               lua_tostring(stageState, -1));
  }
  lua_getglobal(stageState, "configure");
  pushStageBuilder(stageState);
  if (lua_pcall(stageState, 1, 0, 0) != 0) {
    luaL_error(stageState, "error calling stage function: 'configure': %s",
               lua_tostring(stageState, -1));
  }

  try {
    crawlFiles(stageState, stageFilename);
  } catch (InvalidLuaFilenameException *e) {
    delete engine;
    throw e;
  }
  
  int numStageShips = stage->getStageShipCount();
  char **stageShipFilenames = stage->getStageShips();
  lua_getfield(stageState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) (numStageShips + lua_objlen(stageState, -1));
  int cmdLen = numStageShips * 3; // one space, two quotes for each filename
  char **packFilenames = new char*[numFiles];
  for (int x = 0; x < numStageShips; x++) {
    char *stageShipFilename = stageShipFilenames[x];
    bool dup = false;
    for (int y = 0; y < x && !dup; y++) {
      if (strcmp(packFilenames[y], stageShipFilename) == 0) {
        packFilenames[x] = 0;
        dup = true;
      }
    }
    if (!dup) {
      int lenFilename = (int) strlen(stageShipFilename);
      cmdLen += lenFilename;
      packFilenames[x] = new char[lenFilename + 1];
      strcpy(packFilenames[x], stageShipFilename);
    }
  }

  try {
    packageCommon(stageState, stageDir, stageFilename, stageCwd, version,
        STAGE_METAFILE, numStageShips, numFiles, cmdLen, packFilenames, tmpDir,
        nosrc);
  } catch (InvalidLuaFilenameException *e) {
    delete stage;
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;

    throw e;
  }
  
  delete stage;
  for (int x = 0; x < numFiles; x++) {
    delete packFilenames[x];
  }
  delete packFilenames;
}

void FileManager::packageBot(char *botArg, const char *version,
    const char *cacheDir, const char *tmpDir, bool nosrc)
    throw (FileNotFoundException*, InvalidLuaFilenameException*) {
  lua_State *shipState;
  char *shipDir;
  char *shipFilename;
  char *shipCwd;
  loadBotFile(botArg, &shipDir, &shipFilename, &shipCwd, cacheDir);
  checkLuaFilename(shipFilename);
  initShipState(&shipState, shipCwd, shipFilename);
  crawlFiles(shipState, shipFilename);
  
  lua_getfield(shipState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = (int) lua_objlen(shipState, -1);
  char **packFilenames = new char*[numFiles];

  try {
    packageCommon(shipState, shipDir, shipFilename, shipCwd, version,
                  BOT_METAFILE, 0, numFiles, 0, packFilenames, tmpDir, nosrc);
  } catch (InvalidLuaFilenameException *e) {
    for (int x = 0; x < numFiles; x++) {
      delete packFilenames[x];
    }
    delete packFilenames;

    throw e;
  }
  
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

void FileManager::mkdir(const char *filename) {
  char *mkdirCmd = new char[strlen(filename) + 7];
  sprintf(mkdirCmd, "mkdir %s", filename);
  system(mkdirCmd);
  delete mkdirCmd;
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

char* FileManager::parseDir(const char *dirAndFilename) {
  const char *fromFinalSlash = strrchr(dirAndFilename, BB_DIRSEP_CHR);
  if (fromFinalSlash == 0) {
    return 0;
  } else {
    long dirLen = fromFinalSlash - dirAndFilename;
    char *dir = new char[dirLen + 1];
    strncpy(dir, dirAndFilename, dirLen);
    dir[dirLen] = '\0';
    return dir;
  }
}

void FileManager::createDirIfNecessary(const char *dir) {
  if (!fileExists(dir)) {
    mkdir(dir);
  }
}

void FileManager::mkdirIfNecessary(char *dir) {
  char *parentDir = parseDir(dir);
  if (parentDir != 0) {
    mkdirIfNecessary(parentDir);
    delete parentDir;
  }
  if (!fileExists(dir)) {
    mkdir(dir);
  }
}

void FileManager::checkLuaFilename(const char *filename)
    throw (InvalidLuaFilenameException*) {
  if (!isLuaFilename(filename)) {
    throw new InvalidLuaFilenameException(filename);
  }
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
