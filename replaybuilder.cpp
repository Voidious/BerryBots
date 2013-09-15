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

#include <math.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "filemanager.h"
#include "bbutil.h"
#include "basedir.h"
#include "replaybuilder.h"

ReplayBuilder::ReplayBuilder(const char *templateDir) {
  numTeams_ = 0;
  numShips_ = 0;
  shipsAlive_ = shipsShowName_ = 0;
  stagePropertiesData_ = new ReplayData(1);
  wallsData_ = new ReplayData(MAX_MISC_CHUNKS);
  zonesData_ = new ReplayData(MAX_MISC_CHUNKS);
  teamPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipAddData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipRemoveData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipShowNameData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipHideNameData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipTickData_ = new ReplayData(MAX_SHIP_TICK_CHUNKS);
  laserStartData_ = new ReplayData(MAX_LASER_CHUNKS);
  laserEndData_ = new ReplayData(MAX_LASER_CHUNKS);
  laserSparkData_ = new ReplayData(MAX_LASER_CHUNKS);
  torpedoStartData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoEndData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoBlastData_ = new ReplayData(MAX_MISC_CHUNKS);
  torpedoDebrisData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipDestroyData_ = new ReplayData(MAX_MISC_CHUNKS);
  textData_ = new ReplayData(MAX_TEXT_CHUNKS);
  numTexts_ = 0;
  logData_ = new ReplayData(MAX_TEXT_CHUNKS);
  numLogEntries_ = 0;
  resultsData_ = new ReplayData(MAX_MISC_CHUNKS);

  if (templateDir == 0) {
    templatePath_ = 0;
    kineticResourcePath_ = 0;
    replayJsResourcePath_ = 0;
  } else {
    FileManager fileManager;
    templatePath_ = fileManager.getFilePath(templateDir, REPLAY_TEMPLATE);
    kineticResourcePath_ = fileManager.getFilePath(templateDir, KINETIC_JS);
    replayJsResourcePath_ = fileManager.getFilePath(templateDir, BBREPLAY_JS);
  }
}

ReplayBuilder::~ReplayBuilder() {
  delete shipsAlive_;
  delete shipsShowName_;
  delete stagePropertiesData_;
  delete wallsData_;
  delete zonesData_;
  delete teamPropertiesData_;
  delete shipPropertiesData_;
  delete shipAddData_;
  delete shipRemoveData_;
  delete shipShowNameData_;
  delete shipHideNameData_;
  delete shipTickData_;
  delete laserStartData_;
  delete laserEndData_;
  delete laserSparkData_;
  delete torpedoStartData_;
  delete torpedoEndData_;
  delete torpedoBlastData_;
  delete torpedoDebrisData_;
  delete shipDestroyData_;
  delete textData_;
  delete resultsData_;
  if (templatePath_ != 0) {
    delete templatePath_;
  }
  if (kineticResourcePath_ != 0) {
    delete kineticResourcePath_;
  }
  if (replayJsResourcePath_ != 0) {
    delete replayJsResourcePath_;
  }
}

void ReplayBuilder::initShips(int numTeams, int numShips) {
  numTeams_ = numTeams;
  numShips_ = numShips;
  shipsAlive_ = new bool[numShips];
  shipsShowName_ = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    shipsAlive_[x] = shipsShowName_[x] = false;
  }
}

// Stage properties format:  (variable)
// name length | name | width | height
void ReplayBuilder::addStageProperties(const char *name, int width,
                                       int height) {
  int nameLength = (int) strlen(name);
  stagePropertiesData_->addInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    stagePropertiesData_->addInt((int) name[x]);
  }
  stagePropertiesData_->addInt(width);
  stagePropertiesData_->addInt(height);
}

// Wall format:  (4)
// left | bottom | width | height
void ReplayBuilder::addWall(int left, int bottom, int width, int height) {
  wallsData_->addInt(left);
  wallsData_->addInt(bottom);
  wallsData_->addInt(width);
  wallsData_->addInt(height);
}

// Zone format:  (4)
// left | bottom | width | height
void ReplayBuilder::addZone(int left, int bottom, int width, int height) {
  zonesData_->addInt(left);
  zonesData_->addInt(bottom);
  zonesData_->addInt(width);
  zonesData_->addInt(height);
}

// Team properties format:  (variable)
// team index | name length | name
void ReplayBuilder::addTeamProperties(Team *team) {
  teamPropertiesData_->addInt(team->index);
  const char *name = team->name;
  int nameLength = (int) strlen(name);
  teamPropertiesData_->addInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    teamPropertiesData_->addInt((int) name[x]);
  }
}

// Ship properties format:  (variable)
// team index | ship R | G | B | laser R | G | B | thruster R | G | B | name length | name
void ReplayBuilder::addShipProperties(Ship *ship) {
  shipPropertiesData_->addInt(ship->teamIndex);
  ShipProperties *properties = ship->properties;
  shipPropertiesData_->addInt(properties->shipR);
  shipPropertiesData_->addInt(properties->shipG);
  shipPropertiesData_->addInt(properties->shipB);
  shipPropertiesData_->addInt(properties->laserR);
  shipPropertiesData_->addInt(properties->laserG);
  shipPropertiesData_->addInt(properties->laserB);
  shipPropertiesData_->addInt(properties->thrusterR);
  shipPropertiesData_->addInt(properties->thrusterG);
  shipPropertiesData_->addInt(properties->thrusterB);
  const char *name = ship->properties->name;
  int nameLength = (int) strlen(name);
  shipPropertiesData_->addInt(nameLength);
  for (int x = 0; x < nameLength; x++) {
    shipPropertiesData_->addInt((int) name[x]);
  }
}

// Ship add format:  (2)
// ship index | time
void ReplayBuilder::addShip(int shipIndex, int time) {
  shipAddData_->addInt(shipIndex);
  shipAddData_->addInt(time);
}

// Ship remove format:  (2)
// ship index | time
void ReplayBuilder::removeShip(int shipIndex, int time) {
  shipRemoveData_->addInt(shipIndex);
  shipRemoveData_->addInt(time);
}

// Ship show name format:  (2)
// ship index | time
void ReplayBuilder::addShipShowName(int shipIndex, int time) {
  shipShowNameData_->addInt(shipIndex);
  shipShowNameData_->addInt(time);
}

// Ship hide name format:  (2)
// ship index | time
void ReplayBuilder::addShipHideName(int shipIndex, int time) {
  shipHideNameData_->addInt(shipIndex);
  shipHideNameData_->addInt(time);
}

// Ship tick format:  (5)
// x * 10 | y * 10 | thruster angle * 100 | force * 100 | energy * 10
void ReplayBuilder::addShipStates(Ship **ships, int time) {
  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    if (shipsAlive_[x] != ship->alive) {
      if (ship->alive) {
        addShip(ship->index, time);
      } else {
        removeShip(ship->index, time);
      }
      shipsAlive_[x] = ship->alive;
    }
    if (shipsShowName_[x] != ship->showName) {
      if (ship->showName) {
        addShipShowName(ship->index, time);
      } else {
        addShipHideName(ship->index, time);
      }
      shipsShowName_[x] = ship->showName;
    }
  }

  for (int x = 0; x < numShips_; x++) {
    Ship *ship = ships[x];
    if (ship->alive) {
      shipTickData_->addInt(round(ship->x * 10));
      shipTickData_->addInt(round(ship->y * 10));
      shipTickData_->addInt(
          round(normalAbsoluteAngle(ship->thrusterAngle) * 100));
      shipTickData_->addInt(round(limit(0, ship->thrusterForce, 1) * 100));
      shipTickData_->addInt(round(std::max(0.0, ship->energy) * 10));
    }
  }
}

// Laser start format:  (6)
// laser ID | ship index | fire time | x * 10 | y * 10 | heading * 100
void ReplayBuilder::addLaserStart(Laser *laser) {
  laserStartData_->addInt(laser->id);
  laserStartData_->addInt(laser->shipIndex);
  laserStartData_->addInt(laser->fireTime);
  laserStartData_->addInt(round(laser->srcX * 10));
  laserStartData_->addInt(round(laser->srcY * 10));
  laserStartData_->addInt(round(laser->heading * 100));
}

// Laser end format:  (2)
// laser ID | end time
void ReplayBuilder::addLaserEnd(Laser *laser, int time) {
  laserEndData_->addInt(laser->id);
  laserEndData_->addInt(time);
}

// Laser spark format:  (6)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100
void ReplayBuilder::addLaserSpark(Laser *laser, int time, double x, double y,
                                   double dx, double dy) {
  laserSparkData_->addInt(laser->shipIndex);
  laserSparkData_->addInt(time);
  laserSparkData_->addInt(round(x * 10));
  laserSparkData_->addInt(round(y * 10));
  laserSparkData_->addInt(round(dx * 100));
  laserSparkData_->addInt(round(dy * 100));
}

// Torpedo start format:  (6)
// torpedo ID | ship index | fire time | x * 10 | y * 10 | heading * 100 
void ReplayBuilder::addTorpedoStart(Torpedo *torpedo) {
  torpedoStartData_->addInt(torpedo->id);
  torpedoStartData_->addInt(torpedo->shipIndex);
  torpedoStartData_->addInt(torpedo->fireTime);
  torpedoStartData_->addInt(round(torpedo->srcX * 10));
  torpedoStartData_->addInt(round(torpedo->srcY * 10));
  torpedoStartData_->addInt(round(torpedo->heading * 100));
}

// Torpedo end format:  (2)
// torpedo ID | end time
void ReplayBuilder::addTorpedoEnd(Torpedo *torpedo, int time) {
  torpedoEndData_->addInt(torpedo->id);
  torpedoEndData_->addInt(time);
}

// Torpedo blast format:  (3)
// time | x * 10 | y * 10
void ReplayBuilder::addTorpedoBlast(Torpedo *torpedo, int time) {
  torpedoBlastData_->addInt(time);
  torpedoBlastData_->addInt(round(torpedo->x * 10));
  torpedoBlastData_->addInt(round(torpedo->y * 10));
}

// Torpedo debris format:  (7)
// ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
void ReplayBuilder::addTorpedoDebris(Ship *ship, int time, double dx,
                                      double dy, int parts) {
  torpedoDebrisData_->addInt(ship->index);
  torpedoDebrisData_->addInt(time);
  torpedoDebrisData_->addInt(round(ship->x * 10));
  torpedoDebrisData_->addInt(round(ship->y * 10));
  torpedoDebrisData_->addInt(round(dx * 100));
  torpedoDebrisData_->addInt(round(dy * 100));
  torpedoDebrisData_->addInt(parts);
}

// Ship destroy format:  (4)
// ship index | time | x * 10 | y * 10
void ReplayBuilder::addShipDestroy(Ship *ship, int time) {
  shipDestroyData_->addInt(ship->index);
  shipDestroyData_->addInt(time);
  shipDestroyData_->addInt(round(ship->x * 10));
  shipDestroyData_->addInt(round(ship->y * 10));
}

// Text format:  (variable)
// time | textLength | text | x * 10 | y * 10 | size | text R | G | B | A | duration
void ReplayBuilder::addText(int time, const char *text, double x, double y,
                            int size, RgbaColor textColor, int duration) {
  textData_->addInt(time);
  int textLength = (int) strlen(text);
  textData_->addInt(textLength);
  for (int z = 0; z < textLength; z++) {
    textData_->addInt((int) text[z]);
  }
  textData_->addInt(round(x * 10));
  textData_->addInt(round(y * 10));
  textData_->addInt(size);
  textData_->addInt(textColor.r);
  textData_->addInt(textColor.g);
  textData_->addInt(textColor.b);
  textData_->addInt(textColor.a);
  textData_->addInt(duration);
  numTexts_++;
}

// Log format:  (variable)
// team index | time | message length | message
void ReplayBuilder::addLogEntry(Team *team, int time, const char *logMessage) {
  logData_->addInt((team == 0) ? -1 : team->index);
  logData_->addInt(time);
  int messageLen = (int) strlen(logMessage);
  logData_->addInt(messageLen);
  for (int x = 0; x < messageLen; x++) {
    logData_->addInt((int) logMessage[x]);
  }
  numLogEntries_++;
}

// All results format:  (variable)
// num results | <team results>
//
// Expanded results format:
// num results | <team results>
//   Team result: team index | rank | score * 100 | num stats | stats
//     Stat: key length | key | value * 100
void ReplayBuilder::setResults(Team **rankedTeams, int numTeams) {
  if (resultsData_->getSize() > 0) {
    delete resultsData_;
    resultsData_ = new ReplayData(MAX_MISC_CHUNKS);
  }

  int numTeamsShowingResults = 0;
  for (int x = 0; x < numTeams; x++) {
    if (rankedTeams[x]->result.showResult) {
      numTeamsShowingResults++;
    }
  }

  resultsData_->addInt(numTeamsShowingResults);

  for (int x = 0; x < numTeams; x++) {
    TeamResult *teamResult = &(rankedTeams[x]->result);
    if (teamResult->showResult) {
      addResult(rankedTeams[x]);
    }
  }
}

// Result format:  (variable)
// team index | rank | score * 100 | num stats | stats
void ReplayBuilder::addResult(Team *team) {
  resultsData_->addInt(team->index);

  TeamResult *teamResult = &(team->result);
  resultsData_->addInt(teamResult->rank);
  resultsData_->addInt(round(teamResult->score * 100));

  int numStats = teamResult->numStats;
  ScoreStat **stats = teamResult->stats;
  resultsData_->addInt(numStats);
  for (int x = 0; x < numStats; x++) {
    addStat(stats[x]);
  }
}

// Stat format:  (variable)
// key length | key | value * 100
void ReplayBuilder::addStat(ScoreStat *stat) {
  int keyLength = (int) strlen(stat->key);
  char *key = stat->key;
  resultsData_->addInt(keyLength);
  for (int x = 0; x < keyLength; x++) {
    resultsData_->addInt((int) key[x]);
  }
  resultsData_->addInt(round(stat->value * 100));
}

int ReplayBuilder::round(double f) {
  return floor(f + .5);
}

// Format of saved replay file:
// | replay version
// | stage name length | stage name | stage width | stage height
// | num walls | <walls> | num zones | <zones>
// | num ships | <ship properties> | num ship adds | <ship adds>
// | num ship removes | <ship removes> | num ship ticks | <ship ticks>
// | num laser starts | <laser starts> | num laser ends | <laser ends>
// | num laser sparks | <laser sparks>
// | num torpedo starts | <torpedo starts> | num torpedo ends | <torpedo ends>
// | num torpedo blasts | <torpedo blasts> | num torpedo debris | <torpedo debris>
// | num ship destroys | <ship destroys> | num texts | <texts>
// | num log entries | <log entries> | num results | <team results>
void ReplayBuilder::saveReplay(const char *filename) {
  // TODO: throw exceptions for failing to save replay, don't silently fail

  FileManager fileManager;
  char *replayTemplate;
  try {
    replayTemplate = fileManager.readFile(templatePath_);
  } catch (FileNotFoundException *e) {
    delete e;
    replayTemplate = 0;
  }

  char *kineticJsFilename = fileManager.parseFilename(KINETIC_JS);
  char *kineticPath =
      fileManager.getFilePath(getReplaysDir().c_str(), kineticJsFilename);
  delete kineticJsFilename;

  if (!fileManager.fileExists(kineticPath)) {
    char *kineticJs;
    try {
      kineticJs = fileManager.readFile(kineticResourcePath_);
    } catch (FileNotFoundException *e) {
      delete e;
      kineticJs = 0;
    }
    
    fileManager.writeFile(kineticPath, kineticJs);
    if (kineticJs != 0) {
      delete kineticJs;
    }
  }
  delete kineticPath;

  char *replayJsFilename = fileManager.parseFilename(BBREPLAY_JS);
  char *replayJsPath =
      fileManager.getFilePath(getReplaysDir().c_str(), replayJsFilename);
  delete replayJsFilename;
  
  if (!fileManager.fileExists(replayJsPath)) {
    char *replayJs;
    try {
      replayJs = fileManager.readFile(replayJsResourcePath_);
    } catch (FileNotFoundException *e) {
      delete e;
      replayJs = 0;
    }
    
    fileManager.writeFile(replayJsPath, replayJs);
    if (replayJs != 0) {
      delete replayJs;
    }
  }
  delete replayJsPath;

  if (replayTemplate != 0) {
    std::string replayHtml;
    const char *phStart = strstr(replayTemplate, REPLAY_DATA_PLACEHOLDER);
    if (phStart == NULL) {
      return;
    }

    replayHtml.append(replayTemplate, (phStart - replayTemplate));
    replayHtml.append(buildReplayDataString());
    const char *phEnd = &(phStart[strlen(REPLAY_DATA_PLACEHOLDER)]);
    replayHtml.append(phEnd);

    char *filePath = fileManager.getFilePath(getReplaysDir().c_str(), filename);
    char *absFilename = fileManager.getAbsFilePath(filePath);
    delete filePath;
    fileManager.writeFile(absFilename, replayHtml.c_str());
    delete absFilename;
    delete replayTemplate;
  }
}

std::string ReplayBuilder::buildReplayDataString() {
  std::stringstream dataStream;

  dataStream << std::hex << REPLAY_VERSION;
  dataStream << ':' << stagePropertiesHexString()
             << ':' << wallsData_->toHexString(4)
             << ':' << zonesData_->toHexString(4)
             << ':' << teamPropertiesHexString()
             << ':' << shipPropertiesHexString()
             << ':' << shipAddData_->toHexString(2)
             << ':' << shipRemoveData_->toHexString(2)
             << ':' << shipShowNameData_->toHexString(2)
             << ':' << shipHideNameData_->toHexString(2)
             << ':' << shipTickData_->toHexString(5)
             << ':' << laserStartData_->toHexString(6)
             << ':' << laserEndData_->toHexString(2)
             << ':' << laserSparkData_->toHexString(6)
             << ':' << torpedoStartData_->toHexString(6)
             << ':' << torpedoEndData_->toHexString(2)
             << ':' << torpedoBlastData_->toHexString(3)
             << ':' << torpedoDebrisData_->toHexString(7)
             << ':' << shipDestroyData_->toHexString(4)
             << ':' << textDataHexString()
             << ':' << logDataHexString()
             << ':' << resultsDataHexString();

  return dataStream.str();
}

std::string ReplayBuilder::stagePropertiesHexString() {
  std::stringstream hexStream;
  int i = 0;

  int nameLength = stagePropertiesData_->getInt(i++);
  std::stringstream nameStream;
  for (int y = 0; y < nameLength; y++) {
    nameStream << (char) stagePropertiesData_->getInt(i++);
  }
  hexStream << escapeColons(nameStream.str());

  for (int x = 0; x < 2; x++) {
    appendHex(hexStream, stagePropertiesData_->getInt(i++));
  }
  
  return hexStream.str();
}

std::string ReplayBuilder::teamPropertiesHexString() {
  std::stringstream hexStream;
  hexStream << std::hex << numTeams_;

  int i = 0;
  for (int x = 0; x < numTeams_; x++) {
    appendHex(hexStream, teamPropertiesData_->getInt(i++));
    int nameLength = teamPropertiesData_->getInt(i++);
    std::stringstream nameStream;
    for (int y = 0; y < nameLength; y++) {
      nameStream << (char) teamPropertiesData_->getInt(i++);
    }
    hexStream << ':' << escapeColons(nameStream.str());
  }

  return hexStream.str();
}

std::string ReplayBuilder::shipPropertiesHexString() {
  std::stringstream hexStream;
  hexStream << std::hex << numShips_;

  int i = 0;
  char *rgbString = new char[8]; // "#RRGGBB\0"
  for (int x = 0; x < numShips_; x++) {
    appendHex(hexStream, shipPropertiesData_->getInt(i++));
    for (int y = 0; y < 3; y++) {
      int r = shipPropertiesData_->getInt(i++);
      int g = shipPropertiesData_->getInt(i++);
      int b = shipPropertiesData_->getInt(i++);
      sprintf(rgbString, "#%02x%02x%02x", r, g, b);
      hexStream << ':' << rgbString;
    }

    int nameLength = shipPropertiesData_->getInt(i++);
    std::stringstream nameStream;
    for (int y = 0; y < nameLength; y++) {
      nameStream << (char) shipPropertiesData_->getInt(i++);
    }
    hexStream << ':' << escapeColons(nameStream.str());
  }
  delete rgbString;

  return hexStream.str();
}

std::string ReplayBuilder::textDataHexString() {
  std::stringstream hexStream;
  hexStream << std::hex << numTexts_;

  int i = 0;
  char *rgbString = new char[8]; // "#RRGGBB\0"
  for (int x = 0; x < numTexts_; x++) {
    int time = textData_->getInt(i++);
    appendHex(hexStream, time);

    int textLength = textData_->getInt(i++);
    std::stringstream textStream;
    for (int y = 0; y < textLength; y++) {
      textStream << (char) textData_->getInt(i++);
    }
    hexStream << ':' << escapeColons(textStream.str());

    for (int y = 0; y < 3; y++) {
      appendHex(hexStream, textData_->getInt(i++));
    }

    int r = textData_->getInt(i++);
    int g = textData_->getInt(i++);
    int b = textData_->getInt(i++);
    sprintf(rgbString, "#%02x%02x%02x", r, g, b);
    hexStream << ':' << rgbString;

    for (int y = 0; y < 2; y++) {
      appendHex(hexStream, textData_->getInt(i++));
    }
  }
  delete rgbString;

  return hexStream.str();
}

std::string ReplayBuilder::logDataHexString() {
  std::stringstream hexStream;
  hexStream << std::hex << numLogEntries_;
  
  int i = 0;
  for (int x = 0; x < numLogEntries_; x++) {
    for (int y = 0; y < 2; y++) {
      appendHex(hexStream, logData_->getInt(i++));
    }

    int messageLen = logData_->getInt(i++);
    std::stringstream msgStream;
    for (int y = 0; y < messageLen; y++) {
      msgStream << (char) logData_->getInt(i++);
    }
    hexStream << ':' << escapeColons(msgStream.str());
  }
  
  return hexStream.str();
}

// Expanded results format:
// num results | <team results>
//   Team result: team index | rank | score * 100 | num stats | stats
//     Stat: key length | key | value * 100
//
// Encoded to hex format:
// num results : <team results>
//   Team result: team index : rank : score * 100 : num stats : stats
//     Stat: key : value * 100
std::string ReplayBuilder::resultsDataHexString() {
  std::stringstream hexStream;
  int i = 0;
  int numResults = resultsData_->getInt(i++);
  hexStream << std::hex << numResults;
  
  for (int x = 0; x < numResults; x++) {
    for (int y = 0; y < 3; y++) {
      appendHex(hexStream, resultsData_->getInt(i++));
    }

    int numStats = resultsData_->getInt(i++);
    appendHex(hexStream, numStats);
    for (int y = 0; y < numStats; y++) {
      int keyLength = resultsData_->getInt(i++);
      std::stringstream keyStream;
      for (int y = 0; y < keyLength; y++) {
        keyStream << (char) resultsData_->getInt(i++);
      }
      hexStream << ':' << escapeColons(keyStream.str());
      appendHex(hexStream, resultsData_->getInt(i++));
    }
  }
  
  return hexStream.str();
}

std::string ReplayBuilder::escapeColons(std::string s) {
  size_t pos = 0;
  size_t i;
  while ((i = s.find(':', pos)) != std::string::npos) {
    s.replace(i, 0, "\\\\");
    pos = i + 3;
  }
  return s;
}

void ReplayBuilder::appendHex(std::stringstream &hexStream, int i) {
  hexStream << ':';
  int sign = 1;
  if (i < 0) {
    hexStream << '-';
    sign = -1;
  }
  hexStream << std::hex << (sign * i);
}

ReplayData::ReplayData(int maxChunks) {
  chunks_ = new ReplayChunk*[maxChunks];
  maxChunks_ = maxChunks;
  chunks_[0] = new ReplayChunk;
  chunks_[0]->size = 0;
  numChunks_ = 1;
}

ReplayData::~ReplayData() {
  for (int x = 0; x < numChunks_; x++) {
    delete chunks_[x];
  }
  delete chunks_;
}

void ReplayData::addInt(int x) {
  ReplayChunk *chunk = chunks_[numChunks_ - 1];
  if (chunk->size == CHUNK_SIZE) {
    if (numChunks_ == maxChunks_) {
      return;
    }

    chunk = chunks_[numChunks_++] = new ReplayChunk;
    chunk->size = 0;
  }
  chunk->data[chunk->size++] = x;
}

int ReplayData::getSize() {
  return ((numChunks_ - 1) * CHUNK_SIZE) + chunks_[numChunks_ - 1]->size;
}

int ReplayData::getInt(int index) {
  int chunk = index / CHUNK_SIZE;
  int i = index % CHUNK_SIZE;
  return chunks_[chunk]->data[i];
}

void ReplayData::writeChunks(FILE *f) {
  for (int x = 0; x < numChunks_; x++) {
    fwrite(chunks_[x]->data, sizeof(int), chunks_[x]->size, f);
  }
}

std::string ReplayData::toHexString(int blockSize) {
  std::stringstream hexStream;
  if (blockSize > 0) {
    int numElements = getSize() / blockSize;
    hexStream << ':';
    hexStream << std::hex << numElements;
  }
  for (int x = 0; x < numChunks_; x++) {
    int chunkSize = chunks_[x]->size;
    ReplayChunk *chunk = chunks_[x];
    for (int y = 0; y < chunkSize; y++) {
      hexStream << ':';
      int d = chunk->data[y];
      int sign = 1;
      if (d < 0) {
        hexStream << '-';
        sign = -1;
      }
      hexStream << std::hex << (sign * d);
    }
  }
  return hexStream.str().substr(1);
}

ReplayEventHandler::ReplayEventHandler(ReplayBuilder *replayBuilder) {
  replayBuilder_ = replayBuilder;
}

void ReplayEventHandler::handleShipFiredLaser(Ship *firingShip, Laser *laser) {
  replayBuilder_->addLaserStart(laser);
}

void ReplayEventHandler::handleLaserDestroyed(Laser *laser, int time) {
  replayBuilder_->addLaserEnd(laser, time);
}

void ReplayEventHandler::handleShipFiredTorpedo(Ship *firingShip,
                                                Torpedo *torpedo) {
  replayBuilder_->addTorpedoStart(torpedo);
}

void ReplayEventHandler::handleTorpedoDestroyed(Torpedo *torpedo, int time) {
  replayBuilder_->addTorpedoEnd(torpedo, time);
}

void ReplayEventHandler::handleTorpedoExploded(Torpedo *torpedo, int time) {
  replayBuilder_->addTorpedoEnd(torpedo, time);
  replayBuilder_->addTorpedoBlast(torpedo, time);
}

void ReplayEventHandler::handleShipDestroyed(
    Ship *destroyedShip, int time, Ship **destroyerShips, int numDestroyers) {
  replayBuilder_->addShipDestroy(destroyedShip, time);
}

void ReplayEventHandler::handleLaserHitShip(Ship *srcShip, Ship *targetShip,
    Laser *laser, double dx, double dy, int time) {
  replayBuilder_->addLaserSpark(
      laser, time, targetShip->x, targetShip->y, dx, dy);
}

void ReplayEventHandler::handleTorpedoHitShip(Ship *srcShip, Ship *targetShip,
    double dx, double dy, double hitAngle, double hitForce, double hitDamage,
    int time) {
  int parts = ceil((hitDamage / TORPEDO_BLAST_DAMAGE) * MAX_TORPEDO_SPARKS);
  replayBuilder_->addTorpedoDebris(targetShip, time, dx, dy, parts);
}

void ReplayEventHandler::handleStageText(StageText *stageText) {
  RgbaColor textColor;
  textColor.r = stageText->textR;
  textColor.g = stageText->textG;
  textColor.b = stageText->textB;
  textColor.a = stageText->textA;
  
  replayBuilder_->addText(stageText->startTime, stageText->text, stageText->x,
      stageText->y, stageText->fontSize, textColor, stageText->drawTicks);
}
