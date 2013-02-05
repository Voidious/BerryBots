#include "filemanager.h"
#include "basedir.h"

std::string getFullPath(const char *relativePath) {
  char cwd[4096];
  getcwd(cwd, 4096);
  char *absFilename = FileManager::getAbsoluteFilename(cwd, relativePath);
  std::string fullPath = std::string(absFilename);
  delete absFilename;
  return fullPath;
}

std::string getStageDir() {
  return getFullPath("./stages");
}

std::string getBotsDir() {
  return getFullPath("./bots");
}

std::string getCacheDir() {
  return getFullPath("./cache");
}

std::string getTmpDir() {
  return getFullPath("./.tmp");
}
