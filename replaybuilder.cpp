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
  numTeams_ = numTeamsAdded_ = 0;
  numShips_ = 0;
  shipsAlive_ = shipsShowName_ = shipsShowEnergy_ = 0;
  teamNames_ = 0;
  stagePropertiesData_ = new ReplayData(1);
  wallsData_ = new ReplayData(MAX_MISC_CHUNKS);
  zonesData_ = new ReplayData(MAX_MISC_CHUNKS);
  teamPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipPropertiesData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipAddData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipRemoveData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipShowNameData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipHideNameData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipShowEnergyData_ = new ReplayData(MAX_MISC_CHUNKS);
  shipHideEnergyData_ = new ReplayData(MAX_MISC_CHUNKS);
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
  stageName_ = 0;
  timestamp_ = 0;
  if (templateDir == 0) {
    templateDir_ = 0;
  } else {
    templateDir_ = new char[strlen(templateDir) + 1];
    strcpy(templateDir_, templateDir);
  }
}

ReplayBuilder::~ReplayBuilder() {
  if (shipsAlive_ != 0) {
    delete shipsAlive_;
  }
  if (shipsShowName_ != 0) {
    delete shipsShowName_;
  }
  if (shipsShowEnergy_ != 0) {
    delete shipsShowEnergy_;
  }
  if (teamNames_ != 0) {
    for (int x = 0; x < numTeams_; x++) {
      if (teamNames_[x] != 0) {
        delete teamNames_[x];
      }
    }
    delete teamNames_;
  }
  delete stagePropertiesData_;
  delete wallsData_;
  delete zonesData_;
  delete teamPropertiesData_;
  delete shipPropertiesData_;
  delete shipAddData_;
  delete shipRemoveData_;
  delete shipShowNameData_;
  delete shipHideNameData_;
  delete shipShowEnergyData_;
  delete shipHideEnergyData_;
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
  if (timestamp_ != 0) {
    delete timestamp_;
  }
  if (stageName_ != 0) {
    delete stageName_;
  }
  if (templateDir_ != 0) {
   delete templateDir_;
  }
}

void ReplayBuilder::initShips(int numTeams, int numShips) {
  numTeams_ = numTeams;
  numShips_ = numShips;
  shipsAlive_ = new bool[numShips];
  shipsShowName_ = new bool[numShips];
  shipsShowEnergy_ = new bool[numShips];
  for (int x = 0; x < numShips; x++) {
    shipsAlive_[x] = shipsShowName_[x] = shipsShowEnergy_[x] = false;
  }
  teamNames_ = new char*[numTeams];
  for (int x = 0; x < numTeams; x++) {
    teamNames_[x] = 0;
  }
}

// Stage properties format:  (variable)
// name length | name | width | height
void ReplayBuilder::addStageProperties(const char *name, int width,
                                       int height) {
  stageName_ = new char[strlen(name) + 1];
  strcpy(stageName_, name);
  stagePropertiesData_->addString(name);
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
  teamPropertiesData_->addString(team->name);
  int z = numTeamsAdded_++;
  teamNames_[z] = new char[strlen(team->name) + 1];
  strcpy(teamNames_[z], team->name);
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
  shipPropertiesData_->addString(ship->properties->name);
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

// Ship show energy format:  (2)
// ship index | time
void ReplayBuilder::addShipShowEnergy(int shipIndex, int time) {
  shipShowEnergyData_->addInt(shipIndex);
  shipShowEnergyData_->addInt(time);
}

// Ship hide energy format:  (2)
// ship index | time
void ReplayBuilder::addShipHideEnergy(int shipIndex, int time) {
  shipHideEnergyData_->addInt(shipIndex);
  shipHideEnergyData_->addInt(time);
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
    if (shipsShowEnergy_[x] != ship->energyEnabled) {
      if (ship->energyEnabled) {
        addShipShowEnergy(ship->index, time);
      } else {
        addShipHideEnergy(ship->index, time);
      }
      shipsShowEnergy_[x] = ship->energyEnabled;
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
      shipTickData_->addInt(round(std::max(0., ship->energy) * 10));
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
  textData_->addString(text);
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
  logData_->addString(logMessage);
  numLogEntries_++;
}

// All results format:  (variable)
// num results | <results>
//
// Expanded results format:
// num results | <results>
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
  resultsData_->addString(stat->key);
  resultsData_->addInt(round(stat->value * 100));
}

int ReplayBuilder::round(double f) {
  return floor(f + .5);
}

const char* ReplayBuilder::getStageName() {
  return stageName_;
}

void ReplayBuilder::setTimestamp(const char *timestamp) {
  if (timestamp_ != 0) {
    delete timestamp_;
  }
  
  if (timestamp == 0) {
    timestamp_ = 0;
  } else {
    timestamp_ = new char[strlen(timestamp) + 1];
    strcpy(timestamp_, timestamp);
  }
}

void ReplayBuilder::saveReplay(const char *filename) {
  saveReplay(getReplaysDir().c_str(), filename);
}

// Format of saved replay file:
// | replay version
// | stage name length | stage name | stage width | stage height
// | num walls | <walls>
// | num zones | <zones>
// | num ships | <ship properties>
// | num ship adds | <ship adds>
// | num ship removes | <ship removes>
// | num ship show names | <show names>
// | num ship hide names | <hide names>
// | num ship show energys | <show energys>
// | num ship hide energys | <hide energys>
// | num ship ticks | <ship ticks>
// | num laser starts | <laser starts>
// | num laser ends | <laser ends>
// | num laser sparks | <laser sparks>
// | num torpedo starts | <torpedo starts>
// | num torpedo ends | <torpedo ends>
// | num torpedo blasts | <torpedo blasts>
// | num torpedo debris | <torpedo debris>
// | num ship destroys | <ship destroys>
// | num texts | <texts>
// | num log entries | <log entries>
// | num results | <results>
void ReplayBuilder::saveReplay(const char *dir, const char *filename) {
  // TODO: throw exceptions for failing to save replay, don't silently fail

  FileManager fileManager;
  char *replayTemplate = 0;
  char *templatePath = getResourcePath(REPLAY_TEMPLATE);
  if (templatePath != 0) {
    try {
      replayTemplate = fileManager.readFile(templatePath);
    } catch (FileNotFoundException *e) {
      delete e;
    }
    delete templatePath;
  }

  if (replayTemplate != 0) {
    copyReplayResource(KINETIC_JS, dir);
    copyReplayResource(BBREPLAY_JS, dir);

    std::string replayHtml;
    const char *phTitleStart = strstr(replayTemplate, REPLAY_TITLE_PLACEHOLDER);
    if (phTitleStart == NULL) {
      return;
    }
    replayHtml.append(replayTemplate, (phTitleStart - replayTemplate));
    std::stringstream titleStream;
    titleStream << (stageName_ == 0 ? "Unknown" : stageName_);
    titleStream << ": ";
    for (int x = 0; x < numTeams_; x++) {
      char *teamName = teamNames_[x];
      if (teamName != 0) {
        if (x != 0) {
          titleStream << " vs ";
        }
        titleStream << escapeHtml(teamName);
      }
    }
    replayHtml.append(titleStream.str());
    if (timestamp_ != 0) {
      replayHtml.append(" @ ");
      replayHtml.append(timestamp_);
    }
    
    const char *phTitleEnd = &(phTitleStart[strlen(REPLAY_TITLE_PLACEHOLDER)]);
    const char *phDataStart = strstr(phTitleEnd, REPLAY_DATA_PLACEHOLDER);
    if (phDataStart == NULL) {
      return;
    }

    replayHtml.append(phTitleEnd, (phDataStart - phTitleEnd));
    replayHtml.append(buildReplayDataString());
    const char *phEnd = &(phDataStart[strlen(REPLAY_DATA_PLACEHOLDER)]);
    replayHtml.append(phEnd);

    char *filePath = fileManager.getFilePath(dir, filename);
    char *absFilename = fileManager.getAbsFilePath(filePath);
    delete filePath;
    fileManager.writeFile(absFilename, replayHtml.c_str());
    delete absFilename;
    delete replayTemplate;
  }
}

void ReplayBuilder::copyReplayResource(const char *resource,
                                       const char *targetDir) {
  FileManager fileManager;
  char *filename = fileManager.parseFilename(resource);
  char *targetPath = fileManager.getFilePath(targetDir, filename);
  delete filename;

  if (!fileManager.fileExists(targetPath)) {
    char *resourcePath = getResourcePath(resource);
    if (resourcePath != 0) {
      char *s = 0;
      try {
        s = fileManager.readFile(resourcePath);
      } catch (FileNotFoundException *e) {
        delete e;
      }
      
      if (s != 0) {
        fileManager.writeFile(targetPath, s);
        delete s;
      }
      delete resourcePath;
    }
  }
  delete targetPath;
}

std::string ReplayBuilder::buildReplayDataString() {
  std::stringstream dataStream;

  dataStream << std::hex << REPLAY_VERSION;
  dataStream << ':' << stagePropertiesDataString()
             << ':' << wallsData_->toHexString(4)
             << ':' << zonesData_->toHexString(4)
             << ':' << teamPropertiesDataString()
             << ':' << shipPropertiesDataString()
             << ':' << shipAddData_->toHexString(2)
             << ':' << shipRemoveData_->toHexString(2)
             << ':' << shipShowNameData_->toHexString(2)
             << ':' << shipHideNameData_->toHexString(2)
             << ':' << shipShowEnergyData_->toHexString(2)
             << ':' << shipHideEnergyData_->toHexString(2)
             << ':' << shipTickData_->toHexString(5)
             << ':' << laserStartData_->toHexString(6)
             << ':' << laserEndData_->toHexString(2)
             << ':' << laserSparkData_->toHexString(6)
             << ':' << torpedoStartData_->toHexString(6)
             << ':' << torpedoEndData_->toHexString(2)
             << ':' << torpedoBlastData_->toHexString(3)
             << ':' << torpedoDebrisData_->toHexString(7)
             << ':' << shipDestroyData_->toHexString(4)
             << ':' << textDataString()
             << ':' << logDataString()
             << ':' << resultsDataString();

  return dataStream.str();
}

std::string ReplayBuilder::stagePropertiesDataString() {
  std::stringstream out;
  int i = 0;

  appendString(out, stagePropertiesData_, i);
  for (int x = 0; x < 2; x++) {
    appendInt(out, stagePropertiesData_->getInt(i++));
  }
  
  return out.str();
}

std::string ReplayBuilder::teamPropertiesDataString() {
  std::stringstream out;
  out << std::hex << numTeams_;

  int i = 0;
  for (int x = 0; x < numTeams_; x++) {
    appendInt(out, teamPropertiesData_->getInt(i++));
    appendColonString(out, teamPropertiesData_, i);
  }

  return out.str();
}

std::string ReplayBuilder::shipPropertiesDataString() {
  std::stringstream out;
  out << std::hex << numShips_;

  int i = 0;
  char *rgbString = new char[8]; // "#RRGGBB\0"
  for (int x = 0; x < numShips_; x++) {
    appendInt(out, shipPropertiesData_->getInt(i++));
    for (int y = 0; y < 3; y++) {
      int r = shipPropertiesData_->getInt(i++);
      int g = shipPropertiesData_->getInt(i++);
      int b = shipPropertiesData_->getInt(i++);
      sprintf(rgbString, "#%02x%02x%02x", r, g, b);
      out << ':' << rgbString;
    }

    appendColonString(out, shipPropertiesData_, i);
  }
  delete rgbString;

  return out.str();
}

std::string ReplayBuilder::textDataString() {
  std::stringstream out;
  out << std::hex << numTexts_;

  int i = 0;
  char *rgbString = new char[8]; // "#RRGGBB\0"
  for (int x = 0; x < numTexts_; x++) {
    appendInt(out, textData_->getInt(i++));
    appendColonString(out, textData_, i);
    for (int y = 0; y < 3; y++) {
      appendInt(out, textData_->getInt(i++));
    }

    int r = textData_->getInt(i++);
    int g = textData_->getInt(i++);
    int b = textData_->getInt(i++);
    sprintf(rgbString, "#%02x%02x%02x", r, g, b);
    out << ':' << rgbString;

    for (int y = 0; y < 2; y++) {
      appendInt(out, textData_->getInt(i++));
    }
  }
  delete rgbString;

  return out.str();
}

std::string ReplayBuilder::logDataString() {
  std::stringstream out;
  out << std::hex << numLogEntries_;
  
  int i = 0;
  for (int x = 0; x < numLogEntries_; x++) {
    for (int y = 0; y < 2; y++) {
      appendInt(out, logData_->getInt(i++));
    }

    appendColonString(out, logData_, i);
  }
  
  return out.str();
}

// Expanded results format:
// num results | <results>
//   Team result: team index | rank | score * 100 | num stats | stats
//     Stat: key length | key | value * 100
//
// Encoded to hex format:
// num results : <results>
//   Team result: team index : rank : score * 100 : num stats : stats
//     Stat: key : value * 100
std::string ReplayBuilder::resultsDataString() {
  std::stringstream out;
  int i = 0;
  int numResults = resultsData_->getInt(i++);
  out << std::hex << numResults;
  
  for (int x = 0; x < numResults; x++) {
    for (int y = 0; y < 3; y++) {
      appendInt(out, resultsData_->getInt(i++));
    }

    int numStats = resultsData_->getInt(i++);
    appendInt(out, numStats);
    for (int y = 0; y < numStats; y++) {
      appendColonString(out, resultsData_, i);
      appendInt(out, resultsData_->getInt(i++));
    }
  }
  
  return out.str();
}

std::string ReplayBuilder::escapeString(std::string s) {
  findReplace(s, ':', "\\\\:");
  findReplace(s, '\'', "\\'");
  return s;
}

std::string ReplayBuilder::escapeHtml(std::string s) {
  findReplace(s, '&', "&amp;");
  findReplace(s, '<', "&lt;");
  findReplace(s, '>', "&gt;");
  findReplace(s, '"', "&quot;");
  findReplace(s, '\'', "&#039;");
  return s;
}

void ReplayBuilder::findReplace(std::string &s, char find, const char *replace) {
  size_t pos = 0;
  size_t i;
  size_t len = strlen(replace);
  while ((i = s.find(find, pos)) != std::string::npos) {
    s.replace(i, 1, replace);
    pos = i + len;
  }
}

void ReplayBuilder::appendInt(std::stringstream &out, int i) {
  out << ':';
  int sign = 1;
  if (i < 0) {
    out << '-';
    sign = -1;
  }
  out << std::hex << (sign * i);
}

void ReplayBuilder::appendColonString(std::stringstream &out, ReplayData *data,
                                int &i) {
  out << ':';
  appendString(out, data, i);
}

void ReplayBuilder::appendString(std::stringstream &out, ReplayData *data,
                                int &i) {
  int len = data->getInt(i++);
  std::stringstream stringStream;
  for (int x = 0; x < len; x++) {
    stringStream << (char) data->getInt(i++);
  }
  out << escapeString(stringStream.str());
}

char* ReplayBuilder::getResourcePath(const char *resourcePath) {
  if (templateDir_ == 0) {
    return 0;
  }
  FileManager fileManager;
  return fileManager.getFilePath(templateDir_, resourcePath);
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

void ReplayData::addString(const char *s) {
  int len = (int) strlen(s);
  addInt(len);
  for (int x = 0; x < len; x++) {
    addInt((int) s[x]);
  }
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
  std::stringstream out;
  if (blockSize > 0) {
    int numElements = getSize() / blockSize;
    out << ':';
    out << std::hex << numElements;
  }
  for (int x = 0; x < numChunks_; x++) {
    int chunkSize = chunks_[x]->size;
    ReplayChunk *chunk = chunks_[x];
    for (int y = 0; y < chunkSize; y++) {
      out << ':';
      int d = chunk->data[y];
      int sign = 1;
      if (d < 0) {
        out << '-';
        sign = -1;
      }
      out << std::hex << (sign * d);
    }
  }
  return out.str().substr(1);
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
