/*
  Copyright (C) 2012 - Voidious

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
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "bbutil.h"
#include "bbengine.h"
#include "stage.h"
#include "bblua.h"

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

extern BerryBotsEngine *engine;
extern Stage *stage;

int min(int p, int q) {
  if (p < q) {
    return p;
  }
  return q;
}

double min(double p, double q) {
  if (p < q) {
    return p;
  }
  return q;
}

double max(double p, double q) {
  if (p > q) {
    return p;
  }
  return q;
}

double limit(double p, double q, double r) {
  return min(r, max(p, q));
}

int signum(double x) {
  if (x < 0) {
    return -1;
  } else if (x > 0) {
    return 1;
  } else {
    return 0;
  }
}

double square(double x) {
  return x * x;
}

double abs(double x) {
  if (x < 0) {
    return -x;
  }
  return x;
}

double normalRelativeAngle(double x) {
  while (x > M_PI) {
    x -= (2 * M_PI);
  }
  while (x < -M_PI) {
    x += (2 * M_PI);
  }
  return x;
}

double normalAbsoluteAngle(double x) {
  while (x > 2 * M_PI) {
    x -= (2 * M_PI);
  }
  while (x < 0) {
    x += (2 * M_PI);
  }
  return x;
}

double toDegrees(double x) {
  return x * 180 / M_PI;
}

bool fileExists(const char *filename) {
  FILE *testFile = fopen(filename, "r");
  bool exists = (testFile != 0);
  if (exists) {
    fclose(testFile);
  }
  return exists;
}

void mkdir(const char *filename) {
  char *mkdirCmd = new char[strlen(filename) + 7];
  sprintf(mkdirCmd, "mkdir %s", filename);
  system(mkdirCmd);
  delete mkdirCmd;
}

char* parseFilename(const char *dirAndFilename) {
  const char *fromFinalSlash = strrchr(dirAndFilename, '/');
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

char* parseDir(const char *dirAndFilename) {
  const char *fromFinalSlash = strrchr(dirAndFilename, '/');
  if (fromFinalSlash == 0) {
    return 0;
  } else {
    int dirLen = fromFinalSlash - dirAndFilename;
    char *dir = new char[dirLen + 1];
    strncpy(dir, dirAndFilename, dirLen);
    dir[dirLen] = '\0';
    return dir;
  }
}

void createDirIfNecessary(const char *dir) {
  if (!fileExists(dir)) {
    cout << "Creating " << dir << "/ directory ... ";
    mkdir(dir);
    cout << "done!" << endl;
  }
}

void mkdirIfNecessary(char *dir) {
  char *parentDir = parseDir(dir);
  if (parentDir != 0) {
    mkdirIfNecessary(parentDir);
    delete parentDir;
  }
  if (!fileExists(dir)) {
    mkdir(dir);
  }
}

char** parseFlag(int argc, char *argv[], const char *flag, int numValues) {
  for (int x = 0; x < argc - numValues; x++) {
    char *arg = argv[x];
    if (strlen(arg) > 1 && arg[0] == '-' && strcmp(&(arg[1]), flag) == 0) {
      char **values = new char*[numValues];
      for (int y = 0; y < numValues; y++) {
        values[y] = argv[x + y + 1];
      }
      return values;
    }
  }
  return 0;
}

bool flagExists(int argc, char *argv[], const char *flag) {
  for (int x = 0; x < argc; x++) {
    char *arg = argv[x];
    if (strlen(arg) > 1 && arg[0] == '-' && strcmp(&(arg[1]), flag) == 0) {
      return true;
    }
  }
  return false;
}

char* loadUserLuaFilename(char *userDirPath, const char *metaFilename) {
  int userPropertiesPathLen = strlen(userDirPath) + 1 + strlen(metaFilename);
  char *userPropertiesPath = new char[userPropertiesPathLen + 1];
  sprintf(userPropertiesPath, "%s/%s", userDirPath, metaFilename);

  FILE *userPropertiesFile = fopen(userPropertiesPath, "r");
  if (userPropertiesFile == 0) {
    cout << "Failed to find " << userPropertiesPath << endl;
    delete userPropertiesPath;
    return 0;
  }
  delete userPropertiesPath;

  char *userLuaFilename = new char[MAX_FILENAME_LENGTH + 1];
  fgets(userLuaFilename, MAX_FILENAME_LENGTH, userPropertiesFile);
  int userLuaLen = strlen(userLuaFilename);
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

void sliceString(char *filename, int start, int rest) {
  int filenameLen = strlen(filename);
  for (int x = start; x < filenameLen; x++) {
    filename[x] = filename[x + rest - start];
  }
  filename[filenameLen - (rest - start)] = '\0';
}

char* getAbsoluteFilename(char *dir, char *filename) {
  char *absFilename;
  if (filename[0] == '/' || dir == 0) {
    absFilename = new char[strlen(filename) + 1];
    strcpy(absFilename, filename);
    return absFilename;
  }

  absFilename = new char[strlen(dir) + strlen(filename) + 2];
  sprintf(absFilename, "%s/%s", dir, filename);
  char *dots;
  int i = 0;
  while ((dots = strstr(&(absFilename[i]), "./")) != NULL) {
    int offset = dots - absFilename;
    if (offset == 0 || absFilename[offset - 1] == '/') {
      sliceString(absFilename, offset, offset + 2);
      i = offset;
    } else {
      i = offset + 2;
    }
  }
  int filenameLen = strlen(absFilename);
  if (filenameLen >= 2 && strcmp(&(absFilename[filenameLen - 2]), "/.") == 0) {
    absFilename[filenameLen - 2] = '\0';
  }

  while ((dots = strstr(absFilename, "/..")) != 0) {
    int prevSlash = -1;
    for (int x = dots - absFilename - 1; x > 0; x--) {
      if (absFilename[x] == '/') {
        prevSlash = x;
        break;
      }
    }
    if (prevSlash == -1) {
      return absFilename;
    }
    sliceString(absFilename, prevSlash, dots - absFilename + 3);
  }
  return absFilename;
}

void loadUserFile(char *srcFilename, char **userDir, char **userFilename,
    char **userCwd, const char *metaFilename, const char *cacheDir) {
  int srcFilenameLen = strlen(srcFilename);
  int zipLen = strlen(ZIP_EXTENSION);
  if (srcFilenameLen > zipLen
      && strcmp(&(srcFilename[srcFilenameLen - zipLen]), ZIP_EXTENSION) == 0) {
    if (!fileExists(srcFilename)) {
      cout << "ERROR: Couldn't find file: " << srcFilename << endl;
      exit(0);
    }
    createDirIfNecessary(cacheDir);

    const char *userDirName = parseFilename(srcFilename);
    int userDirPathLen = strlen(cacheDir) + 1 + strlen(userDirName);

    char *userDirPath = new char[userDirPathLen + 1];
    sprintf(userDirPath, "%s/%s", cacheDir, userDirName);
    *userDir = userDirPath;

    if (!fileExists(userDirPath)) {
      cout << "Extracting " << srcFilename << " to " << userDirPath << " ... ";
      mkdir(userDirPath);
      int extractCmdLen = 20 + srcFilenameLen + 4 + userDirPathLen;
      char *extractCmd = new char[extractCmdLen + 1];
      sprintf(
          extractCmd, "tar xfv %s -C %s > /dev/null", srcFilename, userDirPath);
      system(extractCmd);
      delete extractCmd;
      cout << "done!" << endl;
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
    if ((*userDir)[0] == '/') {
      *userCwd = new char[strlen(*userDir) + 1];
      strcpy(*userCwd, *userDir);
    } else {
      *userCwd = getAbsoluteFilename(cwd, *userDir);
    }
  }
}

void loadStageFile(char *srcFilename, char **stageDir, char **stageFilename,
    char **stageCwd, const char *cacheDir) {
  loadUserFile(
      srcFilename, stageDir, stageFilename, stageCwd, STAGE_FILENAME, cacheDir);
}

void loadBotFile(char *srcFilename, char **botDir, char **botFilename,
    char **botCwd, const char *cacheDir) {
  loadUserFile(
      srcFilename, botDir, botFilename, botCwd, BOT_FILENAME, cacheDir);
}

bool hasExtension(const char *filename, const char *extension) {
  return ((strlen(filename) > strlen(extension))
          && (strncmp(&(filename[strlen(filename) - strlen(extension)]),
                      extension, strlen(extension)) == 0));
}

bool isLuaFilename(const char *filename) {
  return hasExtension(filename, LUA_EXTENSION);
}

bool isZipFilename(const char *filename) {
  return hasExtension(filename, ZIP_EXTENSION);
}

void checkLuaFilename(const char *filename) {
  if (!isLuaFilename(filename)) {
    cout << "Invalid Lua filename: " << filename << endl;
    exit(0);
  }
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
void packageCommon(lua_State *userState, char *userDir, char *userFilename,
    char *luaCwd, char *version, const char *metaFilename, int prevFiles,
    int numFiles, int prevCmdLen, char **packFilenames, const char *tmpDir,
    bool nosrc) {
  int x = prevFiles;
  lua_pushnil(userState);
  int cmdLen = prevCmdLen;
  while (lua_next(userState, -2) != 0) {
    const char *loadedFilename = lua_tostring(userState, -1);
    checkLuaFilename(loadedFilename);
    int lenFilename = strlen(loadedFilename);
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
        sprintf(outputFilename, "%s/%s", tmpDir, packFilenames[x]);
  
        char *outputDir = parseDir(outputFilename);
        if (outputDir != 0) {
          mkdirIfNecessary(outputDir);
        }
        saveBytecode(packFilenames[x], outputFilename, userDir);
        cout << "Compiled: " << packFilenames[x] << endl;
  
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

  int baseLen = strlen(userFilename) - strlen(LUA_EXTENSION);
  int outputFilenameLen =
      baseLen + 1 + strlen(version) + strlen(ZIP_EXTENSION);
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

  for (int x = 0; x < numFiles; x++) {
    if (packFilenames[x] != 0) {
      printf("Added: %s\n", packFilenames[x]);
    }
  }
  cout << "Packaged " << userFilename << " => " << outputFilename << endl;

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

void packageStage(char *stageArg, char *version, const char *cacheDir,
    const char *tmpDir, bool nosrc) {
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
  crawlFiles(stageState, stageFilename);

  int numStageShips = stage->getStageShipCount();
  char **stageShipFilenames = stage->getStageShips();
  lua_getfield(stageState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = numStageShips + lua_objlen(stageState, -1);
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
      checkLuaFilename(stageShipFilename);
      int lenFilename = strlen(stageShipFilename);
      cmdLen += lenFilename;
      packFilenames[x] = new char[lenFilename + 1];
      strcpy(packFilenames[x], stageShipFilename);
    }
  }

  packageCommon(stageState, stageDir, stageFilename, stageCwd, version,
      STAGE_FILENAME, numStageShips, numFiles, cmdLen, packFilenames, tmpDir,
      nosrc);

  delete stage;
  for (int x = 0; x < numFiles; x++) {
    delete packFilenames[x];
  }
  delete packFilenames;
}

void packageBot(char *botArg, char *version, const char *cacheDir,
    const char *tmpDir, bool nosrc) {
  lua_State *shipState;
  char *shipDir;
  char *shipFilename;
  char *shipCwd;
  loadBotFile(botArg, &shipDir, &shipFilename, &shipCwd, cacheDir);
  checkLuaFilename(shipFilename);
  initShipState(&shipState, shipCwd, shipFilename);  
  crawlFiles(shipState, shipFilename);

  lua_getfield(shipState, LUA_REGISTRYINDEX, "__FILES");
  int numFiles = lua_objlen(shipState, -1);
  char **packFilenames = new char*[numFiles];

  packageCommon(shipState, shipDir, shipFilename, shipCwd, version,
      BOT_FILENAME, 0, numFiles, 0, packFilenames, tmpDir, nosrc);

  for (int x = 0; x < numFiles; x++) {
    delete packFilenames[x];
  }
  delete packFilenames;
}
