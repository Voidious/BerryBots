#ifndef BASE_DIR_H
#define BASE_DIR_H

#include <string>

extern bool isConfigured();
extern std::string getStageDir(void);
extern std::string getBotsDir(void);
extern std::string getCacheDir(void);
extern std::string getTmpDir(void);
extern std::string getApidocPath(void);
extern void setRootDir(std::string newRootDir);
extern void chooseNewRootDir();

#endif
