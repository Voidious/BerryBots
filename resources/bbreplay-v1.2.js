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

// Replay JS version is separate from the app version. The major version is the
// replay data format (aka BerryBots.PLAYER_VERSION). Any replay JS for the same
// data format is cross-compatible, so you can e.g. overwrite v1.3 with v1.9.

// Replay format:
//
// Stage properties:
//   name | width | height
// Wall:
//   left | bottom | width | height
// Zone:
//   left | bottom | width | height
// Team properties:
//   index | name
// Ship properties:
//   team index | ship color | laser color | thruster color | name
// Ship add:
//   ship index | time
// Ship remove:
//   ship index | time
// Ship show name:
//   ship index | time
// Ship hide name:
//   ship index | time
// Ship tick:
//   x * 10 | y * 10 | thruster angle * 100 | force * 100 | energy * 10
// Laser start:
//   laser ID | ship index | fire time | x * 10 | y * 10 | heading * 100
// Laser end:
//   laser ID | end time
// Laser spark:
//   ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100
// Torpedo start:
//   torpedo ID | ship index | fire time | x * 10 | y * 10 | heading * 100
// Torpedo end:
//   torpedo ID | end time
// Torpedo blast:
//   time | x * 10 | y * 10
// Torpedo debris:
//   ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
// Ship destroy:
//   ship index | time | x * 10 | y * 10
// Text:
//   time | text | x * 10 | y * 10 | size | text color | text opacity | duration
// Log entry:
//   team index | time | message
// Result:
//   team index | rank | score * 100 | num stats | stats
// Stat:
//   key | value * 100
//
// Complete file:
// | replay version
// | stage name | stage width | stage height
// | num walls | <walls>
// | num zones | <zones>
// | num teams | <team properties>
// | num ships | <ship properties>
// | num ship adds | <ship adds>
// | num ship removes | <ship removes>
// | num ship show names | <ship show names>
// | num ship hide names | <ship hide names>
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

Kinetic.pixelRatio = 1;

var BerryBots = {
  PLAYER_VERSION: 1,
  STAGE_MARGIN: 25,
  LASER_SPEED: 25,
  TORPEDO_SPEED: 12,
  SHIP_ROTATION: Math.PI * 2 / 240,
  SPINNER_ROTATION: Math.PI / 16,
  THRUSTER_ZERO: 8.3 / 19.3,
  TWO_PI: 2 * Math.PI,
  ZONE_COLOR: '#644444',
  TORPEDO_COLOR: '#ff5926',
  SPARK_LENGTH: 8,
  SPARK_THICKNESS: 1.5,
  SPARK_TIME: 8,
  SPARK_SPEED: 5,
  SPARK_INITIAL_OFFSET: 9,
  TORPEDO_RADIUS: 4,
  BLAST_RADIUS: 100,
  BLAST_TIME: 20,
  DEBRIS_RADIUS: 1.5,
  DEBRIS_TIME: 40,
  DEBRIS_INITIAL_OFFSET: 9,
  DESTROY_BASE_RADIUS: 6,
  DESTROY_CIRCLES: 3,
  DESTROY_FRAMES: 16,
  DESTROY_FRAME_LENGTH: 2,
  DESTROY_TIME: 32 /* 16 * 2 */,
  replayData: BerryBots.replayData,
  interactive: BerryBots.interactive,
  oversizedControls: false,
  initialized: false
};

BerryBots.style = {};
BerryBots.style.consoleTabs =
      '<style type="text/css">'
    + '  .console-tab {'
    + '    font-size: 1.25em;'
    + '    border: 1px solid #fff;'
    + '    padding: 5px;'
    + '    margin: 4px;'
    + '    color: #fff;'
    + '    background-color: #000;'
    + '  } '
    + '  .console-tab:hover {'
    + '    color: #0f0;'
    + '    cursor: pointer;'
    + '    border-color: #0f0;'
    + '    }'
    + '  .stage-tab { margin-bottom: 0.9em; }'
    + '  .results-tab { margin-top: 0.9em; }'
    + '</style>';

BerryBots.style.controls =
      '<style type="text/css">'
    + '  .controls {'
    + '    color: #fff;'
    + '    padding: 5px;'
    + '  }'
    + '  .controls:hover {'
    + '    color: #0f0;'
    + '    cursor: pointer;'
    + '    border-color: #fff;'
    + '  }'
    + '  .play:hover {'
    + '    border-left-color: #0f0;'
    + '  }'
    + '  .pause:hover .pause-wedge {'
    + '    background-color: #0f0;'
    + '  }'
    + '  .speed {'
    + '    position: absolute;'
    + '    color: #fff;'
    + '    font-size: 1.3em;'
    + '  }'
    + '  .speed > div:hover {'
    + '    cursor: pointer;'
    + '  }'
    + '  .restart {'
    + '    position: absolute;'
    + '    font-size: 3.5em;'
    + '    transform:rotate(90deg);'
    + '    -ms-transform:rotate(90deg);'
    + '    -webkit-transform:rotate(90deg);'
    + '  }'
    + '  .restart:hover {'
    + '    color: #0f0;'
    + '    cursor: pointer;'
    + '  }'
    + '</style>';

  BerryBots.style.playPause =
      '<style type="text/css">'
    + '  .play {'
    + '    display: block;'
    + '    height: 0;'
    + '    width: 0;'
    + '    border-left: 60px solid #fff;'
    + '    border-bottom: 60px solid transparent;'
    + '    border-top: 60px solid transparent;'
    + '    transform: scale(2,1);'
    + '    -ms-transform: scale(2,1);'
    + '    -webkit-transform: scale(2,1);'
    + '  }'
    + '  .pause-wedge {'
    + '    background-color: #fff;'
    + '    display: inline-block;'
    + '    height: 115px;'
    + '    width: 30px;'
    + '    margin: 0 15px;'
    + '  }'
    + '</style>';

  BerryBots.style.oversizedPlayPause =
      '<style type="text/css">'
    + '  .play {'
    + '    display: block;'
    + '    height: 0;'
    + '    width: 0;'
    + '    border-left: 80px solid #fff;'
    + '    border-bottom: 80px solid transparent;'
    + '    border-top: 80px solid transparent;'
    + '    transform: scale(2,1);'
    + '    -ms-transform: scale(2,1);'
    + '    -webkit-transform: scale(2,1);'
    + '  }'
    + '  .pause-wedge {'
    + '    background-color: #fff;'
    + '    display: inline-block;'
    + '    height: 165px;'
    + '    width: 40px;'
    + '    margin: 0 20px;'
    + '  }'
    + '</style>';

BerryBots.style.timeDisplay = function(timerWidth, timerLeft, top) {
  return  '<style type="text/css">'
        + '  .timeline {'
        + '    background-color: #333;'
        + '    height: 4px;'
        + '    width: ' + timerWidth + 'px;'
        + '    left: 20px;'
        + '    position: relative;'
        + '    top: 20px;'
        + '  }'
        + '  .timeknob {'
        + '    background: #999;'
        + '    width: 36px;'
        + '    height: 36px;'
        + '    border-radius: 50%;'
        + '    position: relative;'
        + '  }'
        + '  .timetarget {'
        + '    height: 60px;'
        + '    width: ' + (timerWidth + 40) + 'px;'
        + '    left: ' + (timerLeft - 40) + 'px;'
        + '    position: absolute;'
        + '    top: ' + (top - 20) + 'px;'
        + '    padding: 20px;'
        + '    cursor: pointer;'
        + '  }'
        + '  .timetarget:hover .timeline, .timetarget:hover .timeknob {'
        + '    background: #0f0;'
        + '  }'
        + '</style>';
};

BerryBots.desktopPing = function() {
  if (!(BerryBots.initializedControls)) {
    BerryBots.oversizedControls = false;
    BerryBots.initializedControls = true;
  }
  var d = new Date();
  BerryBots.hideOverlayTime =
      Math.max(BerryBots.hideOverlayTime, d.getTime() + 600);
}

BerryBots.touchPing = function() {
  if (!(BerryBots.initializedControls)) {
    BerryBots.oversizedControls = true;
    BerryBots.initializedControls = true;
  }
  var d = new Date();
  BerryBots.hideOverlayTime =
      Math.max(BerryBots.hideOverlayTime, d.getTime() + 2000);
}

BerryBots.getShipProto = function() {
  var shipCircle = new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: 8.15,
    fill: 'black',
    stroke: 'white',
    strokeWidth: 1.7
  });

  var shipDot1 = new Kinetic.Circle({
    x: 0,
    y: -3,
    radius: 1.46,
    fill: '#00ff00',
    strokeWidth: 0
  });

  var shipDot2 = new Kinetic.Circle({
    x: -2.845,
    y: 1.6425,
    radius: 1.46,
    fill: '#00ff00',
    strokeWidth: 0
  });

  var shipDot3 = new Kinetic.Circle({
    x: 2.845,
    y: 1.6425,
    radius: 1.46,
    fill: '#00ff00',
    strokeWidth: 0
  });

  var shipThruster = new Kinetic.Rect({
    x: 0,
    y: 0,
    width: 12,
    height: 6,
    fill: '#ff0000',
    strokeWidth: 0,
    offset: [-8.5, 3]
  });

  var shipEnergy = new Kinetic.Rect({
    x: -15,
    y: 16,
    width: 30,
    height: 2,
    fill: '#ffff00',
    strokeWidth: 0
  });

  var shipName = new Kinetic.Text({
    x: 0,
    y: 16,
    text: '',
    fontSize: 16,
    fontFamily: 'Ubuntu, Arial, Tahoma, sans-serif',
    fill: '#e0e0e0'
  });

  var shipDotProto = new Kinetic.Group({x: 0, y: 0});
  shipDotProto.add(shipDot1);
  shipDotProto.add(shipDot2);
  shipDotProto.add(shipDot3);

  var shipProto = new Kinetic.Group({x: 0, y: 0});
  shipProto.add(shipThruster);
  shipProto.add(shipName);
  shipProto.add(shipEnergy);
  shipProto.add(shipCircle);
  shipProto.add(shipDotProto);

  return shipProto;
};

BerryBots.getShipDestroyProto = function() {
  return new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.DESTROY_BASE_RADIUS,
    strokeWidth: 2
  });
};

BerryBots.getLaserProto = function() {
  return new Kinetic.Rect({
    x: 0,
    y: 0,
    width: 25,
    height: 2,
    fill: '#00ff00',
    strokeWidth: 0,
    offset: [0, 1]
  });
};

BerryBots.getLaserSparkProto = function() {
  return new Kinetic.Rect({
    x: 0,
    y: 0,
    width: BerryBots.SPARK_LENGTH,
    height: BerryBots.SPARK_THICKNESS,
    fill: '#00ff00',
    strokeWidth: 0,
    offset: [0, BerryBots.SPARK_THICKNESS / 2]
  });
};

BerryBots.getTorpedoProto = function() {
  var torpedoCircle = new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.TORPEDO_RADIUS,
    fill: BerryBots.TORPEDO_COLOR,
    strokeWidth: 0
  });

  var torpedoRay = new Kinetic.Rect({
    x: 0,
    y: 0,
    width: BerryBots.TORPEDO_RADIUS * 4,
    height: 1,
    fill: BerryBots.TORPEDO_COLOR,
    offset: [BerryBots.TORPEDO_RADIUS * 2, 0.5]
  });

  var torpedoProto = new Kinetic.Group({x: 0, y: 0});
  torpedoProto.add(torpedoCircle);
  var ray1 = torpedoRay.clone();
  ray1.setRotation(Math.PI / 4);
  var ray2 = torpedoRay.clone();
  ray2.setRotation(-Math.PI / 4);
  torpedoProto.add(ray1);
  torpedoProto.add(ray2);

  return torpedoProto;
};

BerryBots.getTorpedoBlastProto = function() {
  var blastFullCircle = new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.BLAST_RADIUS,
    stroke: BerryBots.TORPEDO_COLOR,
    strokeWidth: 2.5
  });
  var blastInnerCircle = blastFullCircle.clone();

  var torpedoBlastProto = new Kinetic.Group({x: 0, y: 0});
  torpedoBlastProto.add(blastFullCircle);
  torpedoBlastProto.add(blastInnerCircle);
  return torpedoBlastProto;
};

BerryBots.getTorpedoDebrisProto = function() {
  return new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.DEBRIS_RADIUS,
    strokeWidth: 0
  });
};

BerryBots.drawBackground = function() {
  var bgRect = new Kinetic.Rect({
    x: 0,
    y: 0,
    width: BerryBots.canvasWidth,
    height: BerryBots.canvasHeight,
    fill: 'black',
  });
  BerryBots.stageLayer.add(bgRect);
};

BerryBots.getStage = function() {
  return new Kinetic.Stage({
    container: 'container',
    width: BerryBots.canvasWidth,
    height: BerryBots.canvasHeight
  });
};

BerryBots.getStageProperties = function() {
  BerryBots.stageName = BerryBots.getString(1);
  BerryBots.stageWidth = BerryBots.getNumber(2);
  BerryBots.stageHeight = BerryBots.getNumber(3);
  var displayWidth = BerryBots.stageWidth + (BerryBots.STAGE_MARGIN * 2);
  var displayHeight = BerryBots.stageHeight + (BerryBots.STAGE_MARGIN * 2);
  BerryBots.canvasScale = Math.min(1,
      Math.min(window.innerWidth / displayWidth,
               window.innerHeight / displayHeight));
  BerryBots.canvasWidth = displayWidth * BerryBots.canvasScale;
  BerryBots.canvasHeight = displayHeight * BerryBots.canvasScale;
  BerryBots.stage = BerryBots.getStage();
};

BerryBots.scale = function(x) {
  return BerryBots.canvasScale * x;
};

BerryBots.drawRectangles = function(baseOffset, fillColor, numDowns) {
  var numRectangles = BerryBots.getNumber(baseOffset);
  for (var x = 0; x < numRectangles; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var left = BerryBots.getNumber(offset);
    var bottom = BerryBots.getNumber(offset + 1);
    var width = BerryBots.getNumber(offset + 2);
    var height = BerryBots.getNumber(offset + 3);
    var rect = new Kinetic.Rect({
      x: BerryBots.scale(BerryBots.STAGE_MARGIN + left),
      y: BerryBots.scale(
          BerryBots.STAGE_MARGIN + BerryBots.stageHeight - height - bottom),
      width: BerryBots.scale(width),
      height: BerryBots.scale(height),
      fill: fillColor
    });
    BerryBots.stageLayer.add(rect);
    for (var y = 0; y < numDowns; y++) {
      rect.moveDown();
    }
  }
  return numRectangles;
};

BerryBots.drawWallsAndZones = function() {
  var numWalls = BerryBots.drawRectangles(4, 'white', 0);
  var zonesOffset = 5 + (numWalls * 4);
  var numZones =
      BerryBots.drawRectangles(zonesOffset, BerryBots.ZONE_COLOR, numWalls);
  return zonesOffset + 1 + (numZones * 4);
};

BerryBots.getShipOrbits = function(numShips) {
  var shipOrbits = new Array();
  var numShips = BerryBots.ships.length;
  for (var x = 0; x < numShips; x++) {
    shipOrbits[x] = (Math.random() < 0.5 ? 1 : -1);
  }
  return shipOrbits;
};

BerryBots.getTeamProperties = function(baseOffset) {
  var numTeams = BerryBots.getNumber(baseOffset);
  var teams = new Array();
  for (var x = 0; x < numTeams; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var team = new Object();
    team.index = BerryBots.getNumber(offset);
    team.name = BerryBots.getString(offset + 1);
    team.logMessages = new Array();
    team.div = null;
    team.showing = false;
    team.consoleId = 'console' + (team.index + 1);
    teams.push(team);
  }
  return teams;
};

BerryBots.getShipProperties = function(baseOffset) {
  var numShips = BerryBots.getNumber(baseOffset);
  var ships = new Array();
  for (var x = 0; x < numShips; x++) {
    var offset = baseOffset + 1 + (x * 5);
    var ship = BerryBots.shipProto.clone();

    ship.teamIndex = BerryBots.getNumber(offset);
    ship.getChildren()[0].setFill(BerryBots.getString(offset + 3));
    var shipName = ship.getChildren()[1];
    shipName.setText(BerryBots.getString(offset + 4));
    shipName.setOffset({x: shipName.getWidth() / 2 });

    ship.getChildren()[3].setStroke(BerryBots.getString(offset + 1));
    var shipDots = ship.getChildren()[4].getChildren();
    shipDots[0].setFill(BerryBots.getString(offset + 2));
    shipDots[1].setFill(BerryBots.getString(offset + 2));
    shipDots[2].setFill(BerryBots.getString(offset + 2));
    ship.shipColor = BerryBots.getString(offset + 1);
    ship.laserColor = BerryBots.getString(offset + 2);

    ships.push(ship);
    BerryBots.mainLayer.add(ship);
  }
  return ships;
};

BerryBots.parseTeamsAndShips = function(teamsOffset) {
  BerryBots.teams = BerryBots.getTeamProperties(teamsOffset);

  var shipsOffset = teamsOffset + 1 + (BerryBots.teams.length * 2);
  BerryBots.ships = BerryBots.getShipProperties(shipsOffset);
  var numShips = BerryBots.ships.length;
  BerryBots.shipOrbits = BerryBots.getShipOrbits(numShips);
  return shipsOffset + 1 + (numShips * 5);
};

BerryBots.parseReplayVersion = function() {
  BerryBots.values = BerryBots.replayData.replace(/\\:/g, '@;@').split(':');
  BerryBots.replayVersion = BerryBots.getNumber(0);
};

BerryBots.parseGlobalReplayData = function() {
  BerryBots.drawBackground();
  var offset = BerryBots.drawWallsAndZones();
  offset = BerryBots.parseTeamsAndShips(offset);
  BerryBots.stage.add(BerryBots.stageLayer);
  BerryBots.stage.add(BerryBots.mainLayer);
  return offset;
};

BerryBots.parseDynamicReplayData = function(shipAddsOffset) {
  BerryBots.shipAdds = BerryBots.getShipStateChanges(shipAddsOffset);

  var shipRemovesOffset = shipAddsOffset + 1 + (BerryBots.shipAdds.length * 2);
  BerryBots.shipRemoves = BerryBots.getShipStateChanges(shipRemovesOffset);

  var shipAddShowNamesOffset =
      shipRemovesOffset + 1 + (BerryBots.shipRemoves.length * 2);
  BerryBots.shipAddShowNames =
      BerryBots.getShipStateChanges(shipAddShowNamesOffset);

  var shipAddHideNamesOffset =
      shipAddShowNamesOffset + 1 + (BerryBots.shipAddShowNames.length * 2);
  BerryBots.shipAddHideNames =
      BerryBots.getShipStateChanges(shipAddHideNamesOffset);

  var shipAddShowEnergysOffset =
      shipAddHideNamesOffset + 1 + (BerryBots.shipAddHideNames.length * 2);
  BerryBots.shipAddShowEnergys =
      BerryBots.getShipStateChanges(shipAddShowEnergysOffset);

  var shipAddHideEnergysOffset =
      shipAddShowEnergysOffset + 1 + (BerryBots.shipAddShowEnergys.length * 2);
  BerryBots.shipAddHideEnergys =
      BerryBots.getShipStateChanges(shipAddHideEnergysOffset);

  var shipStatesOffset =
      shipAddHideEnergysOffset + 1 + (BerryBots.shipAddHideEnergys.length * 2);
  BerryBots.shipStates = BerryBots.getShipStates(shipStatesOffset);

  var laserStartsOffset =
      shipStatesOffset + 1 + (BerryBots.shipStates.length * 5);
  BerryBots.laserStarts = BerryBots.getProjectileStarts(laserStartsOffset);

  var laserEndsOffset =
      laserStartsOffset + 1 + (BerryBots.laserStarts.length * 6);
  BerryBots.laserEnds = BerryBots.getProjectileEnds(laserEndsOffset);

  var laserSparksOffset =
      laserEndsOffset + 1 + (BerryBots.laserEnds.length * 2);
  BerryBots.laserSparks = BerryBots.getLaserSparks(laserSparksOffset);

  var torpedoStartsOffset =
      laserSparksOffset + 1 + (BerryBots.laserSparks.length * 6);
  BerryBots.torpedoStarts = BerryBots.getProjectileStarts(torpedoStartsOffset);

  var torpedoEndsOffset =
      torpedoStartsOffset + 1 + (BerryBots.torpedoStarts.length * 6);
  BerryBots.torpedoEnds = BerryBots.getProjectileEnds(torpedoEndsOffset);

  var torpedoBlastsOffset =
      torpedoEndsOffset + 1 + (BerryBots.torpedoEnds.length * 2);
  BerryBots.torpedoBlasts = BerryBots.getTorpedoBlasts(torpedoBlastsOffset);

  var torpedoDebrisOffset =
      torpedoBlastsOffset + 1 + (BerryBots.torpedoBlasts.length * 3);
  BerryBots.torpedoDebris = BerryBots.getTorpedoDebris(torpedoDebrisOffset);

  var shipDestroysOffset =
      torpedoDebrisOffset + 1 + (BerryBots.torpedoDebris.length * 7);
  BerryBots.shipDestroys = BerryBots.getShipDestroys(shipDestroysOffset);

  var stageTextsOffset = shipDestroysOffset + 1
      + ((BerryBots.shipDestroys.length / BerryBots.DESTROY_CIRCLES) * 4);
  BerryBots.stageTexts = BerryBots.getStageTexts(stageTextsOffset);

  var logEntriesOffset =
      stageTextsOffset + 1 + (BerryBots.stageTexts.length * 8);
  BerryBots.logEntries = BerryBots.getLogEntries(logEntriesOffset);

  var resultsOffset = logEntriesOffset + 1 + (BerryBots.logEntries.length * 3);
  BerryBots.results = BerryBots.getResults(resultsOffset);
};

BerryBots.parseReplayData = function() {
  BerryBots.parseReplayVersion();
  if (BerryBots.PLAYER_VERSION != BerryBots.replayVersion) {
    window.alert("Error: Replay version: " + BerryBots.replayVersion
        + ", Player version: " + BerryBots.PLAYER_VERSION);
    return;
  }

  BerryBots.getStageProperties();
  BerryBots.initGfx();
  var offset = BerryBots.parseGlobalReplayData();
  BerryBots.parseDynamicReplayData(offset);
};

// Initialize KineticJS graphics.
BerryBots.initGfx = function() {
  if (BerryBots.interactive == null) {
    BerryBots.interactive = true;
  }
  BerryBots.mainLayer = new Kinetic.Layer();
  BerryBots.stageLayer = new Kinetic.Layer();
  BerryBots.spinnerShip = null;
  BerryBots.layers = [BerryBots.stageLayer, BerryBots.mainLayer];

  var scale = BerryBots.canvasScale;
  BerryBots.shipProto = BerryBots.getShipProto();
  BerryBots.shipProto.setScale(scale, scale);
  BerryBots.shipDestroyProto = BerryBots.getShipDestroyProto();
  BerryBots.shipDestroyProto.setScale(scale, scale);
  BerryBots.laserProto = BerryBots.getLaserProto();
  BerryBots.laserProto.setScale(scale, scale);
  BerryBots.laserPool = [];
  BerryBots.laserSparkProto = BerryBots.getLaserSparkProto();
  BerryBots.laserSparkProto.setScale(scale, scale);
  BerryBots.laserSparkPool = [];
  BerryBots.torpedoBlastProto = BerryBots.getTorpedoBlastProto();
  BerryBots.torpedoBlastProto.setScale(scale, scale);
  BerryBots.torpedoBlastPool = [];
  BerryBots.torpedoProto = BerryBots.getTorpedoProto();
  BerryBots.torpedoProto.setScale(scale, scale);
  BerryBots.torpedoPool = [];
  BerryBots.torpedoDebrisProto = BerryBots.getTorpedoDebrisProto();
  BerryBots.torpedoDebrisProto.setScale(scale, scale);
  BerryBots.torpedoDebrisPool = [];
  BerryBots.textPool = [];
  BerryBots.playbackSpeed = 0.5; // ~30 fps
  BerryBots.playbackLabel = 'normal';
};

BerryBots.addBodyListeners = function() {
  if (BerryBots.PLAYER_VERSION != BerryBots.replayVersion) {
    return;
  }

  window.addEventListener('load', function() {
    document.body.onmousemove = function(e) {
      BerryBots.desktopPing();
      if (BerryBots.timeDragging) {
        BerryBots.mouseX = e.pageX;
        BerryBots.clearSelection();
      }
    };

    document.body.onmouseup = function(e) {
      BerryBots.desktopPing();
      BerryBots.timeMouseUp(e);
    };

    document.body.onkeydown = function(e) {
      if (e.which == 27) { // escape
        var numTeams = BerryBots.teams.length;
        for (var x = -1; x < numTeams; x++) {
          var console = BerryBots.getConsole(x);
          if (console.showing) {
            BerryBots.hideConsole(x);
          }
        }

        BerryBots.hideResults();
        BerryBots.timeDragging = false;
        BerryBots.stopSkipping();
      } else if (e.which == 32) { // space bar
        BerryBots.desktopPing();
        BerryBots.playPause();
      }
    };

    document.body.addEventListener('touchstart', function(e) {
      if (BerryBots.timeDragging) {
        BerryBots.mouseX = e.changedTouches[0].pageX;
      }
      BerryBots.touchPing();
      BerryBots.adjustTimeKnob();
    });
  });

  document.body.style.fontFamily = 'Ubuntu, Arial, Tahoma, sans-serif';
};

// Thanks ankur singh:
// http://stackoverflow.com/questions/6562727/is-there-a-function-to-deselect-all-text-using-javascript
BerryBots.clearSelection = function() {
  if (document.selection) {
    document.selection.empty();
  } else if (window.getSelection) {
    window.getSelection().removeAllRanges();
  }
};

BerryBots.scrollToBottom = function(d) {
  d.scrollTop = d.scrollHeight;
};

BerryBots.getConsole = function(teamIndex) {
  if (teamIndex == -1) {
    return BerryBots.stageConsole;
  } else {
    return BerryBots.teams[teamIndex];
  }
};

BerryBots.hideConsole = function(teamIndex) {
  var console = BerryBots.getConsole(teamIndex);
  document.getElementById('container').removeChild(console.div);
  console.showing = false;
};

BerryBots.toggleConsole = function(teamIndex) {
  var numTeams = BerryBots.teams.length;
  for (var x = -1; x < numTeams; x++) {
    var console = BerryBots.getConsole(x);
    if (x != teamIndex && console.showing) {
      BerryBots.hideConsole(x);
    }
    if (x == teamIndex && console.showing) {
      BerryBots.hideConsole(x);
      return;
    }
  }

  var console = BerryBots.getConsole(teamIndex);
  var consoleId = console.consoleId;
  if (console.div == null) {
    var s = '<style type="text/css">.console { border-collapse: collapse;'
        + 'border: 1px solid #fff; padding: 0.5em; background-color: #000;'
        + 'font-family: Consolas, Courier, monospace; font-size: 1.0em;'
        + 'color: #fff; overflow: auto; width: '
        + Math.min(window.innerWidth, 725) + 'px; height: 480px; }'
        + '.console-title { font-size: 1.2em; background-color: #fff; '
        + 'color: #000; position: relative; text-align: center; padding: 5px;}'
        + '.console-x { font-size: 2.25em; position: absolute; left: 4px;'
        + 'cursor: pointer; top: -10px; } .console-x:hover { color: #f00; }'
        + '</style>'
        + '<div class="console-title"><div class="console-x" '
        + 'onclick="BerryBots.hideConsole(' + teamIndex + ')">&times;</div>'
        + console.name + '</div><div class="console" id="' + consoleId + '">';

    var messages = console.logMessages;
    var numMessages = messages.length;
    for (var x = 0; x < numMessages; x++) {
      messages[x] = BerryBots.escapeHtml(messages[x]);
    }
    s += messages.join('<br>');
    s += '</div>';
    var d = document.createElement('div');
    d.innerHTML = s;
    d.margin = '0';
    d.padding = '0';
    document.getElementById('container').appendChild(d);
    console.div = d;
    console.outputDiv = document.getElementById(consoleId);
  } else {
    document.getElementById('container').appendChild(console.div);
  }

  var d = console.div;
  d.style.position = 'absolute';
  d.style.left = Math.max(10, ((BerryBots.stage.getScaleX()
      * BerryBots.stage.getWidth()) - 775)) + 'px';
  d.style.top = '35px';

  console.showing = true;
  BerryBots.scrollToBottom(console.outputDiv);
};

BerryBots.clearConsoles = function() {
  var numTeams = BerryBots.teams.length;
  for (var x = -1; x < numTeams; x++) {
    var console = BerryBots.getConsole(x);
    console.logMessages.splice(0, console.logMessages.length);
    if (console.div != null) {
      console.outputDiv.innerHTML = '';
    }
  }
};

BerryBots.toggleResults = function() {
  if (BerryBots.resultsDiv == null) {
    BerryBots.showResults();
  } else {
    BerryBots.hideResults();
  }
};

BerryBots.hideResults = function() {
  if (BerryBots.resultsDiv != null) {
    document.getElementById('container').removeChild(BerryBots.resultsDiv);
    BerryBots.resultsDiv = null;
  }
};

BerryBots.playPause = function() {
  BerryBots.paused = !(BerryBots.paused);
  BerryBots.stopSkipping();
  BerryBots.showPlayPause();
};

BerryBots.restartMatch = function() {
  BerryBots.hideResults();
  BerryBots.clearConsoles();
  BerryBots.initReplayState(BerryBots.replayState);
  BerryBots.showPlayPause();
};

BerryBots.setSpeed = function(speed, label) {
  BerryBots.playbackSpeed = speed;
  BerryBots.playbackLabel = label;
  BerryBots.setPlaybackLabelColors();
};

BerryBots.stopSkipping = function() {
  BerryBots.skipTime = -1;
  if (BerryBots.spinnerShip != null) {
    BerryBots.spinnerShip.destroy();
    BerryBots.spinnerShip = null;
    BerryBots.mainLayer.setOpacity(1);
  }
};

BerryBots.setPlaybackLabelColor = function(d) {
  if (d != null) {
    d.style.color = (d.id == BerryBots.playbackLabel) ? '#0f0' : '#fff';
  }
};

BerryBots.setPlaybackLabelColors = function() {
  BerryBots.setPlaybackLabelColor(document.getElementById('normal'));
  BerryBots.setPlaybackLabelColor(document.getElementById('bouncy'));
  BerryBots.setPlaybackLabelColor(document.getElementById('ludicrous'));
}

BerryBots.showPlayPause = function() {
  var playElement = document.getElementById('play');
  var pauseElement = document.getElementById('pause');
  var speedElement = document.getElementById('speed');
  var restartElement = document.getElementById('restart');
  if (playElement == null) {
    return;
  }

  var playStyle = playElement.style;
  var pauseStyle = pauseElement.style;
  var speedStyle = speedElement.style;
  var restartStyle = restartElement.style;
  var stage = BerryBots.stage;
  var center =
      Math.max(0, (stage.getScaleX() * stage.getWidth()) / 2);
  var top = (stage.getScaleY() * stage.getHeight());
  if (BerryBots.gameTime >= BerryBots.endTime) {
    playStyle.visibility = 'hidden';
    pauseStyle.visibility = 'hidden';
    speedStyle.visibility = 'hidden';
    restartStyle.top = '-180px';
    restartStyle.left = (center - 20) + 'px';
  } else {
    var d = document.getElementById('controls');
    var normalDiv = document.getElementById('normal');
    var bouncyDiv = document.getElementById('bouncy');
    var ludicDiv = document.getElementById('ludicrous');
    speedStyle.position = normalDiv.style.position = bouncyDiv.style.position =
        ludicDiv.style.position = d.style.position = 'absolute';
    speedStyle.visibility = 'visible';
    var bottomMargin = 300 + Math.max(0, (Math.min(400, top - 400) / 20));
    // TODO: streamline these branches
    if (BerryBots.paused) {
      if (BerryBots.oversizedControls) {
        d.style.left = Math.max(0, center - 30) + 'px';
        d.style.top = (top - 355) + 'px';
        normalDiv.style.top = bouncyDiv.style.top = ludicDiv.style.top = '-140px';
        restartStyle.top = '108px';
        restartStyle.left = '-97px';
        normalDiv.style.left = '-112px';
        bouncyDiv.style.left = '-7px';
        ludicDiv.style.left = '93px';
      } else {
        d.style.left = Math.max(0, center - 22) + 'px';
        d.style.top = (top - bottomMargin) + 'px';
        normalDiv.style.top = bouncyDiv.style.top =
            ludicDiv.style.top = '-90px';
        restartStyle.top = '68px';
        restartStyle.left = '-105px';
        normalDiv.style.left = '-120px';
        bouncyDiv.style.left = '-15px';
        ludicDiv.style.left = '85px';
      }
      pauseStyle.visibility = 'hidden';
      playStyle.visibility = 'visible';
    } else {
      if (BerryBots.oversizedControls) {
        d.style.left = Math.max(0, center - 87) + 'px';
        d.style.top = (top - 520) + 'px';
        restartStyle.top = '273px';
        restartStyle.left = '-40px';
        normalDiv.style.left = '-55px';
        bouncyDiv.style.left = '50px';
        ludicDiv.style.left = '150px';
      } else {
        d.style.left = Math.max(0, center - 66) + 'px';
        d.style.top = (top - bottomMargin - 115) + 'px';
        restartStyle.top = '183px';
        restartStyle.left = '-61px';
        normalDiv.style.left = '-76px';
        bouncyDiv.style.left = '29px';
        ludicDiv.style.left = '129px';
      }
      normalDiv.style.top = bouncyDiv.style.top = ludicDiv.style.top = '25px';
      playStyle.visibility = 'hidden';
      pauseStyle.visibility = 'visible';
    }

    BerryBots.setPlaybackLabelColors();
  }
};

BerryBots.showOverlay = function() {
  if (BerryBots.interactive) {
    BerryBots.showConsoleTabs();
    BerryBots.showControls();
    BerryBots.showTimeDisplay();
    BerryBots.showingOverlay = true;
  }
};

BerryBots.hideOverlay = function() {
  var d = document.getElementById('dock');
  document.getElementById('container').removeChild(d);
  var f = document.getElementById('controls');
  document.getElementById('container').removeChild(f);
  var g = document.getElementById('timedisplay');
  document.getElementById('container').removeChild(g);
  BerryBots.knob = null;
  BerryBots.showingOverlay = false;
};

BerryBots.showConsoleTabs = function() {
  var s = BerryBots.style.consoleTabs
      + '<div class="console-tab stage-tab" '
      + 'onclick="BerryBots.toggleConsole(-1)">' + BerryBots.stageName + '</div>'

  var ships = BerryBots.ships;
  var teams = BerryBots.teams;
  var numTeams = teams.length;
  var numShips = ships.length;
  for (var x = 0; x < numTeams; x++) {
    var showName = false;
    for (var y = 0; y < numShips; y++) {
      if (ships[y].teamIndex == x && BerryBots.shipShowNames[y]) {
        showName = true;
        break;
      }
    }
    if (showName) {
      s += '<div class="console-tab" onclick="BerryBots.toggleConsole('
          + teams[x].index + ')">' + BerryBots.escapeHtml(teams[x].name)
          + '</div>';
    }
  }

  s += '<div class="console-tab results-tab" '
      + 'onclick="BerryBots.toggleResults()">Results</div>';

  var d = document.createElement('div');
  d.innerHTML = s;
  d.id = 'dock';
  d.style.color = '#fff';
  d.margin = '0';
  d.padding = '0';
  d.style.position = 'absolute';
  d.style.left = '35px';
  d.style.top = '35px';
  document.getElementById('container').appendChild(d);
};

BerryBots.showControls = function() {
  var s = BerryBots.style.controls
      + (BerryBots.oversizedControls
          ? BerryBots.style.oversizedPlayPause : BerryBots.style.playPause)
      + '<div class="controls">'
      + '  <div id="play" class="play" onclick="BerryBots.playPause()"></div>'
      + '  <div id="pause" class="pause" onclick="BerryBots.playPause()">'
      + '    <div class="pause-wedge"></div>'
      + '    <div class="pause-wedge"></div>'
      + '  </div>'
      + '</div>'
      + '<div id="speed" class="speed">'
      + '  <div id="normal" onclick="BerryBots.setSpeed(0.5, \'normal\')">Normal</div>'
      + '  <div id="bouncy" onclick="BerryBots.setSpeed(1, \'bouncy\')">Bouncy</div>'
      + '  <div id="ludicrous"'
      + '      onclick="BerryBots.setSpeed(20, \'ludicrous\')">Ludicrous</div>'
      + '</div>'
      + '<div id="restart" class="restart" onclick="BerryBots.restartMatch()">&#8635;</div>';

  var d = document.createElement('div');
  d.innerHTML = s;
  d.id = 'controls';
  d.style.color = '#fff';
  d.margin = '0';
  d.padding = '0';
  d.style.position = 'absolute';
  document.getElementById('container').appendChild(d);

  BerryBots.showPlayPause();
};

BerryBots.showTimeDisplay = function() {
  var stage = BerryBots.stage;
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 90);
  var timerWidth = (stageWidth / 2);
  var timerLeft = (stageWidth / 4);
  var s = BerryBots.style.timeDisplay(timerWidth, timerLeft, top)
      + '<div id="timetarget" class="timetarget">'
      + '  <div id="timeline" class="timeline"></div>'
      + '  <div id="timeknob" class="timeknob"></div>'
      + '</div>';

  var d = document.createElement('div');
  d.innerHTML = s;
  d.id = 'timedisplay';
  d.margin = '0';
  d.padding = '0';
  document.getElementById('container').appendChild(d);

  BerryBots.knob = document.getElementById('timeknob');
  var timeTarget = document.getElementById('timetarget');
  timeTarget.onmousedown = function(e) {
    BerryBots.desktopPing();
    BerryBots.timeMouseDown(e);
  };
  timeTarget.addEventListener('touchstart', function(e) {
    BerryBots.touchPing();
    BerryBots.timeDragging = true;
    BerryBots.mouseX = e.changedTouches[0].pageX;
    BerryBots.adjustTimeKnob();
    e.preventDefault();
  });
  timeTarget.addEventListener('touchmove', function(e) {
    BerryBots.touchPing();
    if (BerryBots.timeDragging) {
      BerryBots.mouseX = e.changedTouches[0].pageX;
      BerryBots.clearSelection();
    }
    e.preventDefault();
  });
  timeTarget.addEventListener('touchend', function(e) {
    BerryBots.dragEnd(e.changedTouches[0].pageX);
    e.preventDefault();
  });

  BerryBots.adjustTimeKnob();
};

BerryBots.adjustTimeKnob = function() {
  var knobStyle = BerryBots.knob.style;
  knobStyle.top = '1px';

  var stage = BerryBots.stage;
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var timerWidth = stageWidth / 2;
  if (BerryBots.timeDragging) {
    var left = stageWidth / 4;
    // Visual feedback you can't rewind (for now).
    var minX = (BerryBots.gameTime / BerryBots.endTime) * timerWidth;
    knobStyle.left = (Math.max(minX,
        Math.min(timerWidth, (BerryBots.mouseX - left)))) + 'px';
  } else {
    var left = (BerryBots.gameTime / BerryBots.endTime) * (stageWidth / 2);
    knobStyle.left = left + 'px';
  }
};

BerryBots.timeMouseDown = function(e) {
  BerryBots.timeDragging = true;
  BerryBots.mouseX = e.pageX;
  BerryBots.adjustTimeKnob();
};

BerryBots.timeMouseUp = function(e) {
  if (BerryBots.timeDragging) {
    BerryBots.dragEnd(e.pageX);
  }
};

BerryBots.dragEnd = function(x) {
  BerryBots.timeDragging = false;
  var stage = BerryBots.stage;
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var left = stageWidth / 4;
  var width = stageWidth / 2;
  var skipFraction = (x - left) / width;
  BerryBots.skipTime = Math.max(0,
      Math.min(Math.round(skipFraction * BerryBots.endTime),
          BerryBots.endTime));
  if (BerryBots.skipTime > BerryBots.gameTime && BerryBots.paused) {
    BerryBots.playPause();
  }
};

BerryBots.showResults = function() {
  if (BerryBots.results.length == 0) {
    return;
  }
  if (BerryBots.showingOverlay) {
    BerryBots.hideOverlay();
  }
  BerryBots.hideResults();
  BerryBots.showPlayPause();

  var s = '<style type="text/css">table, td, th { border-collapse: collapse;'
      + 'background-color: #fff; border: 1px solid #555; color: #000;'
      + 'font-size: 1.1em; }'
      + '.num { text-align: right; } .mid { text-align: center; }'
      + '.results-x { font-size: 2.5em; position: absolute; left: 10px;'
      + 'cursor: pointer; top: -3px; } .results-x:hover { color: #f00; }'
      + '.resultsDialog { border: 6px solid #555; border-radius: 6px;'
      + 'margin: 0; padding: 0; display: inline-block; }'
      + '</style>';
  
  var numResults = BerryBots.results.length;
  var hasScores = false;
  var statKeys = new Array();
  var results = BerryBots.results;
  for (var x = 0; x < numResults; x++) {
    var result = results[x];
    hasScores = hasScores || (result.score != 0);
    var stats = result.stats;
    var numStats = stats.length;
    for (var y = 0; y < numStats; y++) {
      var stat = stats[y]
      if (statKeys.indexOf(stat.key) == -1) {
        statKeys.push(stat.key);
      }
    }
  }

  var numKeys = statKeys.length;
  var numCols = 2 + (hasScores ? 1 : 0) + numKeys;
    s += '<div id="resultsDialog" class="resultsDialog">'
        + '<table cellpadding="9px" id="resultsTable">'
        + '<tr><td colspan="' + numCols + '" class="mid">'
        + '<div class="results-x" onclick="BerryBots.hideResults()">&times;'
        + '</div>Results</td></tr>'
        + '<tr><td class="mid">Rank</td>'
        + '<td class="mid">Name</td>';

  if (hasScores) {
    s += '<td class="mid">Score</td>';
  }
  for (var x = 0; x < numKeys; x++) {
    s += '<td class="mid">' + statKeys[x] + '</td>';
  }
  s += '</tr>';

  var teams = BerryBots.teams;
  for (var x = 0; x < numResults; x++) {
    var result = results[x];
    var rank = (result.rank == 0 ? '-' : result.rank);
    s += '<tr><td class="mid">' + rank + '</td><td>'
        + teams[result.teamIndex].name + '</td>'
        + (hasScores ? '<td class="num">' + result.score + '</td>' : '');
    var stats = result.stats;
    var numStats = stats.length;
    for (var y = 0; y < numKeys; y++) {
      var found = false;
      for (var z = 0; z < numStats; z++) {
        if (statKeys[y] == stats[z].key) {
          s += '<td class="num">' + stats[z].value + '</td>';
          found = true;
          break;
        }
      }
      if (!found) {
        s += '<td></td>';
      }
    }
    s += '</tr>'
  }
  s += '</table></div>\n';

  var d = document.createElement('div');
  d.innerHTML = s;
  d.margin = '0';
  d.padding = '0';
  document.getElementById('container').appendChild(d);

  var resultsDialog = document.getElementById('resultsDialog');
  var stage = BerryBots.stage;
  var left = Math.max(0,
      ((stage.getScaleX() * stage.getWidth()) - resultsDialog.offsetWidth) / 2);
  var top = Math.max(0,
      ((stage.getScaleY() * stage.getHeight())
          - resultsDialog.offsetHeight) / 2);
  d.style.position = 'absolute';
  d.style.left = left + 'px';
  d.style.top = top + 'px';
  BerryBots.resultsDiv = d;
};

// Functions for parsing replay data into data model.

BerryBots.getShipStateChanges = function(baseOffset) {
  var numShipStateChanges = BerryBots.getNumber(baseOffset);
  var shipStateChanges = new Array();
  for (var x = 0; x < numShipStateChanges; x++) {
    var offset = baseOffset + 1 + (x * 2);
    shipStateChanges[x] = {index: BerryBots.getNumber(offset),
                           time: BerryBots.getNumber(offset + 1)};
  }
  return shipStateChanges;
}

BerryBots.getShipStates = function(baseOffset) {
  var numTicks = BerryBots.getNumber(baseOffset);
  var shipStates = new Array();
  for (var x = 0; x < numTicks; x++) {
    var offset = baseOffset + 1 + (x * 5);
    var shipX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset) / 10);
    var shipY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 1) / 10));
    var thrusterAngle = BerryBots.getNumber(offset + 2) / 100;
    var thrusterForce = BerryBots.getNumber(offset + 3) / 100;
    var energy = BerryBots.getNumber(offset + 4) / 10;

    shipStates[x] = {x: shipX, y: shipY, thrusterAngle: thrusterAngle,
                     thrusterForce: thrusterForce, energy: energy};
  }
  return shipStates;
}

BerryBots.getProjectileStarts = function(baseOffset) {
  var numProjectileStarts = BerryBots.getNumber(baseOffset);
  var projectileStarts = new Array();
  for (var x = 0; x < numProjectileStarts; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var projectileId = BerryBots.getNumber(offset);
    var shipIndex = BerryBots.getNumber(offset + 1);
    var fireTime = BerryBots.getNumber(offset + 2);
    var srcX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 3) / 10);
    var srcY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 4) / 10));
    var heading = BerryBots.getNumber(offset + 5) / 100;

    projectileStarts[x] = {id: projectileId, shipIndex: shipIndex,
        fireTime: fireTime, srcX: srcX, srcY: srcY, heading: heading};
  }
  return projectileStarts;
}

BerryBots.getProjectileEnds = function(baseOffset) {
  var numProjectileEnds = BerryBots.getNumber(baseOffset);
  var projectileEnds = new Array();
  for (var x = 0; x < numProjectileEnds; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var projectileId = BerryBots.getNumber(offset);
    var endTime = BerryBots.getNumber(offset + 1);
    projectileEnds[x] = {id: projectileId, time: endTime};
  }
  return projectileEnds;
}

BerryBots.getLaserSparks = function(baseOffset) {
  var numLaserSparks = BerryBots.getNumber(baseOffset);
  var laserSparks = new Array();
  for (var x = 0; x < numLaserSparks; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var shipIndex = BerryBots.getNumber(offset);
    var time = BerryBots.getNumber(offset + 1);
    var sparkX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10);
    var sparkY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10));
    var dx = BerryBots.scale(BerryBots.getNumber(offset + 4) / 100);
    var dy = BerryBots.scale(BerryBots.getNumber(offset + 5) / 100);
    laserSparks[x] = {shipIndex: shipIndex, startTime: time, x: sparkX,
        y: sparkY, dx: dx, dy: dy, endTime: time + BerryBots.SPARK_TIME};
  }
  return laserSparks;
}

BerryBots.getTorpedoBlasts = function(baseOffset) {
  var numTorpedoBlasts = BerryBots.getNumber(baseOffset);
  var torpedoBlasts = new Array();
  for (var x = 0; x < numTorpedoBlasts; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var time = BerryBots.getNumber(offset);
    var blastX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 1) / 10);
    var blastY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 2) / 10));
    torpedoBlasts[x] = {startTime: time, x: blastX, y: blastY,
                        endTime: time + BerryBots.BLAST_TIME};
  }
  return torpedoBlasts;
}

BerryBots.getTorpedoDebris = function(baseOffset) {
  var numTorpedoDebris = BerryBots.getNumber(baseOffset);
  var torpedoDebris = new Array();
  for (var x = 0; x < numTorpedoDebris; x++) {
    var offset = baseOffset + 1 + (x * 7);
    var shipIndex = BerryBots.getNumber(offset);
    var time = BerryBots.getNumber(offset + 1);
    var debrisX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10);
    var debrisY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10));
    var dx = BerryBots.scale(BerryBots.getNumber(offset + 4) / 100);
    var dy = BerryBots.scale(BerryBots.getNumber(offset + 5) / 100);
    var parts = BerryBots.getNumber(offset + 6);
    var speeds = new Array();
    for (var y = 0; y < parts; y++) {
      speeds.push(BerryBots.scale(((Math.random() + 1) / 2) * 2.5))
    }
    torpedoDebris[x] = {shipIndex: shipIndex, startTime: time, x: debrisX,
        y: debrisY, dx: dx, dy: dy, parts: parts, speeds: speeds,
        endTime: time + BerryBots.DEBRIS_TIME};
  }
  return torpedoDebris;
}

BerryBots.getShipDestroys = function(baseOffset) {
  var numShipDestroys = BerryBots.getNumber(baseOffset);
  var shipDestroys = new Array();
  for (var x = 0; x < numShipDestroys; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var shipIndex = BerryBots.getNumber(offset);
    var time = BerryBots.getNumber(offset + 1);
    var shipX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10);
    var shipY = BerryBots.scale(BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10));
    var endTime = time + BerryBots.DESTROY_TIME;
    for (var y = 0; y < BerryBots.DESTROY_CIRCLES; y++) {
      var startTime = time + (y * BerryBots.DESTROY_FRAME_LENGTH);
      shipDestroys.push({shipIndex: shipIndex, startTime: startTime, x: shipX,
                         y: shipY, endTime: endTime});
    }
  }
  return shipDestroys;
};

BerryBots.getStageTexts = function(baseOffset) {
  var numStageTexts = BerryBots.getNumber(baseOffset);
  var stageTexts = new Array();
  for (var x = 0; x < numStageTexts; x++) {
    var offset = baseOffset + 1 + (x * 8);
    var time = BerryBots.getNumber(offset);
    var text = BerryBots.getString(offset + 1);
    var size = BerryBots.scale(BerryBots.getNumber(offset + 4));
    var textX = BerryBots.scale(
        BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10);
    var textY = BerryBots.scale(BerryBots.STAGE_MARGIN + BerryBots.stageHeight
        - (BerryBots.getNumber(offset + 3) / 10) - size);
    var color = BerryBots.getString(offset + 5);
    var opacity = BerryBots.getNumber(offset + 6);
    var duration = BerryBots.getNumber(offset + 7);
    var endTime = time + duration;
    stageTexts[x] = {startTime: time, text: text, x: textX, y: textY,
        size: size, color: color, opacity: opacity,
        endTime: time + duration};
  }
  return stageTexts;
};

BerryBots.getLogEntries = function(baseOffset) {
  var numLogEntries = BerryBots.getNumber(baseOffset);
  var logEntries = new Array();
  for (var x = 0; x < numLogEntries; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var teamIndex = BerryBots.getNumber(offset);
    var time = BerryBots.getNumber(offset + 1);
    var message = BerryBots.getString(offset + 2);
    logEntries.push({teamIndex: teamIndex, time: time, message: message});
  }
  return logEntries;
};

BerryBots.getResults = function(baseOffset) {
  var offset = baseOffset;
  var numResults = BerryBots.getNumber(offset++);
  var results = new Array();
  for (var x = 0; x < numResults; x++) {
    var teamIndex = BerryBots.getNumber(offset++);
    var rank = BerryBots.getNumber(offset++);
    var score = BerryBots.getNumber(offset++) / 100;
    var numStats = BerryBots.getNumber(offset++);
    var stats = new Array();
    for (var y = 0; y < numStats; y++) {
      var key = BerryBots.getString(offset++);
      var value = BerryBots.getNumber(offset++) / 100;
      stats.push({key: key, value: value});
    }
    results.push(
        {teamIndex: teamIndex, rank: rank, score: score, stats: stats});
  }
  return results;
};

BerryBots.getNumber = function(offset) {
  return parseInt(BerryBots.values[offset], 16);
};

BerryBots.getString = function(offset) {
  return BerryBots.values[offset].replace(/@;@/g, ':');
};

BerryBots.getEndTime = function(numShips, shipStates, shipAdds, shipRemoves) {
  var alives = new Array();
  for (var x = 0; x < numShips; x++) {
    alives[x] = false;
  }

  var gameTime = 0;
  var nextShipState = 0;
  var nextShipAdd = 0;
  var nextShipRemove = 0;
  var numShipStates = shipStates.length;
  var numShipAdds = shipAdds.length;
  var numShipRemoves = shipRemoves.length;
  while(nextShipState < numShipStates) {
    while (nextShipAdd < numShipAdds
           && shipAdds[nextShipAdd].time <= gameTime) {
      alives[shipAdds[nextShipAdd++].index] = true;
    }
    while (nextShipRemove < numShipRemoves
           && shipRemoves[nextShipRemove].time <= gameTime) {
      alives[shipRemoves[nextShipRemove++].index] = false;
    }

    for (var x = 0; x < numShips; x++) {
      if (alives[x]) {
        nextShipState++;
      }
    }
    gameTime++;
  }
  return gameTime;
};

BerryBots.escapeHtml = function(s) {
  return s.replace(/&/g, "&amp;")
          .replace(/</g, "&lt;")
          .replace(/>/g, "&gt;")
          .replace(/"/g, "&quot;")
          .replace(/'/g, "&#039;")
          .replace(/ /g, "&nbsp;")
          .replace(/\t/g, "&nbsp&nbsp&nbsp&nbsp")
          .replace(/\n/g, "<br>\n");
};

BerryBots.makeLaser = function() {
  var lasers = BerryBots.laserPool;
  var len = lasers.length;
  if (len > 0) {
    var laser = lasers[len - 1];
    lasers.splice(len - 1, 1);
    return laser;
  } else {
    return BerryBots.laserProto.clone();
  }
};

BerryBots.endLaser = function(laser) {
  laser.remove();
  BerryBots.laserPool.push(laser);
};

BerryBots.makeLaserSpark = function() {
  var laserSparks = BerryBots.laserSparkPool;
  var len = laserSparks.length;
  if (len > 0) {
    var spark = laserSparks[len - 1];
    laserSparks.splice(len - 1, 1);
    return spark;
  } else {
    return BerryBots.laserSparkProto.clone();
  }
};

BerryBots.endLaserSpark = function(laserSpark) {
  laserSpark.remove();
  BerryBots.laserSparkPool.push(laserSpark);
};

BerryBots.makeTorpedo = function() {
  var torpedos = BerryBots.torpedoPool;
  var len = torpedos.length;
  if (len > 0) {
    var torpedo = torpedos[len - 1];
    torpedos.splice(len - 1, 1);
    return torpedo;
  } else {
    return BerryBots.torpedoProto.clone();
  }
};

BerryBots.endTorpedo = function(torpedo) {
  torpedo.remove();
  BerryBots.torpedoPool.push(torpedo);
};

BerryBots.makeTorpedoBlast = function() {
  var torpedoBlasts = BerryBots.torpedoBlastPool;
  var len = torpedoBlasts.length;
  if (len > 0) {
    var blast = torpedoBlasts[len - 1];
    torpedoBlasts.splice(len - 1, 1);
    return blast;
  } else {
    return BerryBots.torpedoBlastProto.clone();
  }
};

BerryBots.endTorpedoBlast = function(torpedoBlast) {
  torpedoBlast.remove();
  BerryBots.torpedoBlastPool.push(torpedoBlast);
};

BerryBots.makeTorpedoDebris = function() {
  var torpedoDebrises = BerryBots.torpedoDebrisPool;
  var len = torpedoDebrises.length;
  if (len > 0) {
    var debris = torpedoDebrises[len - 1];
    torpedoDebrises.splice(len - 1, 1);
    return debris;
  } else {
    return BerryBots.torpedoDebrisProto.clone();
  }
};

BerryBots.endTorpedoDebris = function(torpedoDebris) {
  torpedoDebris.remove();
  BerryBots.torpedoDebrisPool.push(torpedoDebris);
};

BerryBots.makeText = function() {
  var texts = BerryBots.textPool;
  var len = texts.length;
  if (len > 0) {
    var text = texts[len - 1];
    texts.splice(len - 1, 1);
    return text;
  } else {
    return new Kinetic.Text({
      fontFamily: 'Questrial, Ubuntu, Arial, Tahoma, sans-serif',
      strokeWidth: 0
    });
  }
};

BerryBots.endText = function(text) {
  text.remove();
  BerryBots.textPool.push(text);
};

BerryBots.initReplayState = function(rs) {
  BerryBots.stopSkipping();
  BerryBots.gameTime = 0;
  BerryBots.skipTime = -1;
  BerryBots.showedResults = false;
  BerryBots.paused = false;
  BerryBots.timeDragging = false;
  BerryBots.nextDrawTime = 0;
  var numShips = BerryBots.ships.length;

  if (rs == undefined) {
    rs = {};
  } else {
    for (var x = 0; x < rs.destroyCircles.length; x++) {
      rs.destroyCircles[x].destroy();
    }
    for (var x = 0; x < rs.lasers.length; x++) {
      BerryBots.endLaser(rs.lasers[x]);
    }
    for (var x = 0; x < rs.sparkRects.length; x++) {
      BerryBots.endLaserSpark(rs.sparkRects[x]);
    }
    for (var x = 0; x < rs.torpedos.length; x++) {
      BerryBots.endTorpedo(rs.torpedos[x]);
    }
    for (var x = 0; x < rs.blastCircles.length; x++) {
      BerryBots.endTorpedoBlast(rs.blastCircles[x]);
    }
    for (var x = 0; x < rs.debrisCircles.length; x++) {
      BerryBots.endTorpedoDebris(rs.debrisCircles[x]);
    }
    for (var x = 0; x < rs.stageTextShapes.length; x++) {
      BerryBots.endText(rs.stageTextShapes[x]);
    }
  }

  rs.shipAlives = new Array();
  rs.shipShowNames = BerryBots.shipShowNames = new Array();
  rs.shipShowEnergys = new Array();
  rs.destroyCircles = new Array();
  rs.lasers = new Array();
  rs.sparkRects = new Array();
  rs.torpedos = new Array();
  rs.blastCircles = new Array();
  rs.debrisCircles = new Array();
  rs.stageTextShapes = new Array();

  for (var x = 0; x < numShips; x++) {
    rs.shipAlives[x] = false;
    rs.shipShowNames[x] = false;
    rs.shipShowEnergys[x] = false;
  }

  rs.nextShipState = 0;
  rs.nextShipAdd = 0;
  rs.nextShipRemove = 0;
  rs.nextShipShowName = 0;
  rs.nextShipHideName = 0;
  rs.nextShipShowEnergy = 0;
  rs.nextShipHideEnergy = 0;
  rs.nextShipDestroy = 0;
  rs.nextLaserStart = 0;
  rs.nextLaserEnd = 0;
  rs.nextLaserSpark = 0;
  rs.nextTorpedoStart = 0;
  rs.nextTorpedoEnd = 0;
  rs.nextTorpedoBlast = 0;
  rs.nextTorpedoDebris = 0;
  rs.nextStageText = 0;
  rs.nextLogEntry = 0;

  return rs;
};

// Replay the match from our data model, tick by tick.
BerryBots.replay = function() {
  if (BerryBots.PLAYER_VERSION != BerryBots.replayVersion) {
    return;
  }

  var ships = BerryBots.ships;
  var numShips = BerryBots.ships.length;
  var rs = BerryBots.replayState = BerryBots.initReplayState();

  BerryBots.stageConsole = new Object();
  BerryBots.stageConsole.logMessages = new Array();
  BerryBots.stageConsole.div = null;
  BerryBots.stageConsole.outputDiv = null;
  BerryBots.stageConsole.showing = false;
  BerryBots.stageConsole.consoleId = 'console0';
  BerryBots.stageConsole.name = BerryBots.stageName;
  BerryBots.knob = null;

  BerryBots.resultsDiv = null;

  BerryBots.hideOverlayTime = 0;
  BerryBots.showingOverlay = false;

  BerryBots.endTime = BerryBots.getEndTime(BerryBots.ships.length,
      BerryBots.shipStates, BerryBots.shipAdds, BerryBots.shipRemoves);
  BerryBots.mouseX = 0;

  var anim = new Kinetic.Animation(function(frame) {
    var d = new Date();
    var now = d.getTime();
    if (now  < BerryBots.hideOverlayTime) {
      if (!(BerryBots.showingOverlay)) {
        BerryBots.showOverlay();
      }
    } else {
      if (BerryBots.showingOverlay) {
        BerryBots.hideOverlay();
      }
    }

    if (BerryBots.showingOverlay) {
      BerryBots.adjustTimeKnob();
    }
    if (!(BerryBots.paused)) {
      if (BerryBots.gameTime >= BerryBots.nextDrawTime) {
        BerryBots.nextDrawTime += BerryBots.playbackSpeed;
      } else {
        if (rs.nextShipState < BerryBots.shipStates.length) {
          do {
            var gameTime = BerryBots.gameTime;
  
            // Lasers.
            var laserStarts = BerryBots.laserStarts;
            var numLaserStarts = laserStarts.length;
            while (rs.nextLaserStart < numLaserStarts
                   && laserStarts[rs.nextLaserStart].fireTime <= gameTime) {
              var laserStart = laserStarts[rs.nextLaserStart++];
              var laser = BerryBots.makeLaser();
              var dx = BerryBots.scale(
                  Math.cos(laserStart.heading) * BerryBots.LASER_SPEED);
              var dy = BerryBots.scale(
                  Math.sin(laserStart.heading) * BerryBots.LASER_SPEED);
              laser.setX(laserStart.srcX);
              laser.setY(laserStart.srcY);
              laser.setFill(ships[laserStart.shipIndex].laserColor);
              laser.setRotation(BerryBots.TWO_PI - laserStart.heading);
              laser.laserData = {dx: dx, dy: dy, id: laserStart.id};
              rs.lasers.push(laser);
              BerryBots.mainLayer.add(laser);
            }
  
            var laserEnds = BerryBots.laserEnds;
            var numLaserEnds = laserEnds.length;
            while (rs.nextLaserEnd < numLaserEnds
                   && laserEnds[rs.nextLaserEnd].time <= gameTime) {
              var laserEnd = laserEnds[rs.nextLaserEnd++];
              // TODO: use a map instead?
              for (var x = 0; x < rs.lasers.length; x++) {
                var laser = rs.lasers[x];
                if (laser.laserData.id == laserEnd.id) {
                  rs.lasers.splice(x--, 1);
                  BerryBots.endLaser(laser);
                }
              }
            }
  
            for (var x = 0; x < rs.lasers.length; x++) {
              var laser = rs.lasers[x];
              laser.setX(laser.getX() + laser.laserData.dx);
              laser.setY(laser.getY() - laser.laserData.dy);
            }
  
            for (var x = 0; x < rs.sparkRects.length; x++) {
              var sparkRect = rs.sparkRects[x];
              if (sparkRect.sparkData.endTime <= gameTime) {
                rs.sparkRects.splice(x--, 1);
                BerryBots.endLaserSpark(sparkRect);
              } else {
                var sparkOffset = sparkRect.getOffset();
                sparkRect.setOffset(
                    {x: sparkOffset.x + BerryBots.scale(BerryBots.SPARK_SPEED),
                     y: sparkOffset.y});
                sparkRect.move(sparkRect.sparkData.dx, -sparkRect.sparkData.dy);
              }
            }
  
            var laserSparks = BerryBots.laserSparks;
            var numLaserSparks = laserSparks.length;
            while (rs.nextLaserSpark < numLaserSparks
                   && laserSparks[rs.nextLaserSpark].startTime <= gameTime) {
              var laserSpark = laserSparks[rs.nextLaserSpark++];
              for (var x = 0; x < 4; x++) {
                var sparkRect = BerryBots.makeLaserSpark();
                sparkRect.setX(laserSpark.x);
                sparkRect.setY(laserSpark.y);
                sparkRect.setFill(ships[laserSpark.shipIndex].laserColor);
                sparkRect.setRotation(Math.random() * BerryBots.TWO_PI);
                sparkRect.setOffset(
                    {x: BerryBots.scale(BerryBots.SPARK_INITIAL_OFFSET),
                     y: BerryBots.scale(BerryBots.SPARK_THICKNESS / 2)});
                sparkRect.sparkData = laserSpark;
                rs.sparkRects.push(sparkRect);
                BerryBots.mainLayer.add(sparkRect);
              }
            }
  
            // Torpedos.
            var torpedoStarts = BerryBots.torpedoStarts;
            var numTorpedoStarts = torpedoStarts.length;
            while (rs.nextTorpedoStart < numTorpedoStarts
                   && torpedoStarts[rs.nextTorpedoStart].fireTime <= gameTime) {
              var torpedoStart = torpedoStarts[rs.nextTorpedoStart++];
              var torpedo = BerryBots.makeTorpedo();
              var dx = BerryBots.scale(
                  Math.cos(torpedoStart.heading) * BerryBots.TORPEDO_SPEED);
              var dy = BerryBots.scale(
                  Math.sin(torpedoStart.heading) * BerryBots.TORPEDO_SPEED);
              torpedo.setX(torpedoStart.srcX);
              torpedo.setY(torpedoStart.srcY);
              torpedo.torpedoData = {dx: dx, dy: dy, id: torpedoStart.id};
              rs.torpedos.push(torpedo);
              BerryBots.mainLayer.add(torpedo);
            }
  
            var torpedoEnds = BerryBots.torpedoEnds;
            var numTorpedoEnds = torpedoEnds.length;
            while (rs.nextTorpedoEnd < numTorpedoEnds
                   && torpedoEnds[rs.nextTorpedoEnd].time <= gameTime) {
              var torpedoEnd = torpedoEnds[rs.nextTorpedoEnd++];
              // TODO: use a map instead?
              for (var x = 0; x < rs.torpedos.length; x++) {
                var torpedo = rs.torpedos[x];
                if (torpedo.torpedoData.id == torpedoEnd.id) {
                  rs.torpedos.splice(x--, 1);
                  BerryBots.endTorpedo(torpedo);
                }
              }
            }
  
            for (var x = 0; x < rs.torpedos.length; x++) {
              var torpedo = rs.torpedos[x];
              torpedo.setX(torpedo.getX() + torpedo.torpedoData.dx);
              torpedo.setY(torpedo.getY() - torpedo.torpedoData.dy);
            }
  
            var torpedoBlasts = BerryBots.torpedoBlasts;
            var numTorpedoBlasts = torpedoBlasts.length;
            while (rs.nextTorpedoBlast < numTorpedoBlasts
                   && torpedoBlasts[rs.nextTorpedoBlast].startTime
                       <= gameTime) {
              var torpedoBlast = torpedoBlasts[rs.nextTorpedoBlast++];
              var blastGroup = BerryBots.makeTorpedoBlast();
              blastGroup.setX(torpedoBlast.x);
              blastGroup.setY(torpedoBlast.y);
              blastGroup.blastData = torpedoBlast;
              rs.blastCircles.push(blastGroup);
              BerryBots.mainLayer.add(blastGroup);
            }
  
            for (var x = 0; x < rs.blastCircles.length; x++) {
              var blastGroup = rs.blastCircles[x];
              if (blastGroup.blastData.endTime <= gameTime) {
                rs.blastCircles.splice(x--, 1);
                BerryBots.endTorpedoBlast(blastGroup);
              } else {
                var blastTime = gameTime - blastGroup.blastData.startTime;
                var blastScale = (blastTime + 1) / BerryBots.BLAST_TIME;
                blastGroup.getChildren()[1].setRadius(
                    blastScale * BerryBots.BLAST_RADIUS);
                blastGroup.getChildren()[0].setVisible(
                    blastTime < 10 && (blastTime <= 2 || blastTime >=7));
              }
            }
  
            for (var x = 0; x < rs.debrisCircles.length; x++) {
              var debrisCircle = rs.debrisCircles[x];
              if (debrisCircle.debrisData.endTime <= gameTime) {
                rs.debrisCircles.splice(x--, 1);
                BerryBots.endTorpedoDebris(debrisCircle);
              } else {
                debrisCircle.setOffset({x: debrisCircle.getOffset().x
                                            + debrisCircle.debrisData.speed});
                debrisCircle.move(debrisCircle.debrisData.dx,
                                  -debrisCircle.debrisData.dy);
              }
            }
  
            var torpedoDebris = BerryBots.torpedoDebris;
            var numTorpedoDebris = torpedoDebris.length;
            while (rs.nextTorpedoDebris < numTorpedoDebris
                   && torpedoDebris[rs.nextTorpedoDebris].startTime <= gameTime) {
              var debrisData = torpedoDebris[rs.nextTorpedoDebris++];
              for (var x = 0; x < debrisData.parts; x++) {
                var debrisCircle = BerryBots.makeTorpedoDebris();
                debrisCircle.setX(debrisData.x);
                debrisCircle.setY(debrisData.y);
                debrisCircle.setFill(ships[debrisData.shipIndex].shipColor);
                debrisCircle.setRotation(Math.random() * BerryBots.TWO_PI);
                debrisCircle.setOffset(
                    {x: BerryBots.scale(BerryBots.DEBRIS_INITIAL_OFFSET)}
                );
                debrisCircle.debrisData = {dx: debrisData.dx, dy: debrisData.dy,
                    speed: debrisData.speeds[x], endTime: debrisData.endTime};
                rs.debrisCircles.push(debrisCircle);
                BerryBots.mainLayer.add(debrisCircle);
              }
            }
  
            // Ship destroys.
            for (var x = 0; x < rs.destroyCircles.length; x++) {
              var destroyCircle = rs.destroyCircles[x];
              if (destroyCircle.destroyData.endTime <= gameTime) {
                rs.destroyCircles.splice(x--, 1);
                destroyCircle.destroy();
              } else {
                var radiusFactor = 1 + Math.floor(
                    (gameTime - destroyCircle.destroyData.startTime) / 2);
                destroyCircle.setRadius(
                    radiusFactor * BerryBots.DESTROY_BASE_RADIUS);
              }
            }
  
            var shipDestroys = BerryBots.shipDestroys;
            var numShipDestroys = shipDestroys.length;
            while (rs.nextShipDestroy < numShipDestroys
                   && shipDestroys[rs.nextShipDestroy].startTime <= gameTime) {
              var shipDestroy = shipDestroys[rs.nextShipDestroy++];
              var destroyCircle = BerryBots.shipDestroyProto.clone();
              destroyCircle.setX(shipDestroy.x);
              destroyCircle.setY(shipDestroy.y);
              destroyCircle.setStroke(ships[shipDestroy.shipIndex].shipColor);
              destroyCircle.destroyData = shipDestroy;
              rs.destroyCircles.push(destroyCircle);
              BerryBots.mainLayer.add(destroyCircle);
            }
  
            // Ships.
            var shipAdds = BerryBots.shipAdds;
            var numShipAdds = shipAdds.length;
            while (rs.nextShipAdd < numShipAdds
                   && shipAdds[rs.nextShipAdd].time <= gameTime) {
              rs.shipAlives[shipAdds[rs.nextShipAdd++].index] = true;
            }
  
            var shipRemoves = BerryBots.shipRemoves;
            var numShipRemoves = shipRemoves.length;
            while (rs.nextShipRemove < numShipRemoves
                   && shipRemoves[rs.nextShipRemove].time <= gameTime) {
              rs.shipAlives[shipRemoves[rs.nextShipRemove++].index] = false;
            }
  
            var shipAddShowNames = BerryBots.shipAddShowNames;
            var numShipAddShowNames = shipAddShowNames.length;
            while (rs.nextShipShowName < numShipAddShowNames
                   && shipAddShowNames[rs.nextShipShowName].time <= gameTime) {
              var shipIndex = shipAddShowNames[rs.nextShipShowName++].index;
              rs.shipShowNames[shipIndex] = true;
            }
  
            var shipAddHideNames = BerryBots.shipAddHideNames;
            var numShipAddHideNames = shipAddHideNames.length;
            while (rs.nextShipHideName < numShipAddHideNames
                   && shipAddHideNames[rs.nextShipHideName].time <= gameTime) {
               var shipIndex = shipAddHideNames[rs.nextShipHideName++].index;
              rs.shipShowNames[shipIndex] = false;
            }
  
            var shipAddShowEnergys = BerryBots.shipAddShowEnergys;
            var numShipAddShowEnergys = shipAddShowEnergys.length;
            while (rs.nextShipShowEnergy < numShipAddShowEnergys
                   && shipAddShowEnergys[rs.nextShipShowEnergy].time
                       <= gameTime) {
              var shipIndex = shipAddShowEnergys[rs.nextShipShowEnergy++].index;
              rs.shipShowEnergys[shipIndex] = true;
              ships[shipIndex].getChildren()[1].setPosition(0, 24);
            }
  
            var shipAddHideEnergys = BerryBots.shipAddHideEnergys;
            var numShipAddHideEnergys = shipAddHideEnergys.length;
            while (rs.nextShipHideEnergy < numShipAddHideEnergys
                   && shipAddHideEnergys[rs.nextShipHideEnergy].time <= gameTime) {
              var shipIndex = shipAddHideEnergys[rs.nextShipHideEnergy++].index;
              rs.shipShowEnergys[shipIndex] = false;
              ships[shipIndex].getChildren()[1].setPosition(0, 16);
            }
  
            for (var x = 0; x < numShips; x++) {
              var ship = ships[x];
              if (rs.shipAlives[x]) {
                var shipState = BerryBots.shipStates[rs.nextShipState++];
                ship.show();
                ship.setX(shipState.x);
                ship.setY(shipState.y);
                var thruster = ship.getChildren()[0];
                thruster.setRotation(
                    Math.PI + (BerryBots.TWO_PI - shipState.thrusterAngle));
                var forceFactor = shipState.thrusterForce;
                if (forceFactor < 0.0001) {
                  thruster.setVisible(false);
                } else {
                  thruster.setVisible(true);
                  thruster.setScale(BerryBots.THRUSTER_ZERO
                      + (forceFactor * (1 - BerryBots.THRUSTER_ZERO)));
                }
  
                var energy = shipState.energy / 100;
                var energyShape = ship.getChildren()[2];
                if (energy < 0.0001) {
                  energyShape.setVisible(false);
                } else {
                  energyShape.setVisible(rs.shipShowEnergys[x]);
                  energyShape.setScale(energy);
                }
                var shipDotGroup = ship.getChildren()[4];
                shipDotGroup.setRotation(shipDotGroup.getRotation()
                    + (BerryBots.SHIP_ROTATION * BerryBots.shipOrbits[x]));
                ship.getChildren()[1].setVisible(rs.shipShowNames[x]);
              } else {
                ship.hide();
              }
            }
  
            // Stage texts.
            for (var x = 0; x < rs.stageTextShapes.length; x++) {
              var stageTextShape = rs.stageTextShapes[x];
              if (stageTextShape.textData.endTime <= gameTime) {
                rs.stageTextShapes.splice(x--, 1);
                BerryBots.endText(stageTextShape);
              }
            }
  
            var stageTexts = BerryBots.stageTexts;
            var numStageTexts = stageTexts.length;
            while (rs.nextStageText < numStageTexts
                   && stageTexts[rs.nextStageText].startTime <= gameTime) {
              var stageText = stageTexts[rs.nextStageText++];
              var stageTextShape = BerryBots.makeText();
              stageTextShape.setPosition(stageText.x, stageText.y);
              stageTextShape.setText(stageText.text);
              stageTextShape.setFontSize(stageText.size);
              stageTextShape.setFill(stageText.color);
  
              stageTextShape.textData = stageText;
              rs.stageTextShapes.push(stageTextShape);
              BerryBots.mainLayer.add(stageTextShape);
            }
  
            // Log entries.
            var logEntries = BerryBots.logEntries;
            var numLogEntries = logEntries.length;
            while (rs.nextLogEntry < numLogEntries
                   && logEntries[rs.nextLogEntry].time <= gameTime) {
              var logEntry = logEntries[rs.nextLogEntry++];
              var console = BerryBots.getConsole(logEntry.teamIndex);
  
              console.logMessages.push(logEntry.message);
              if (console.div != null) {
                var outputDiv = console.outputDiv;
                outputDiv.innerHTML +=
                    ((outputDiv.innerHTML == '') ? '' : '<br>')
                    + BerryBots.escapeHtml(logEntry.message);
                BerryBots.scrollToBottom(console.outputDiv);
              }
            }
  
            BerryBots.gameTime++;
            if (BerryBots.gameTime < BerryBots.skipTime) {
              BerryBots.nextDrawTime = BerryBots.gameTime;
              if (BerryBots.spinnerShip == null) {
                var ship = BerryBots.spinnerShip = BerryBots.shipProto.clone();
                var shipParts = ship.getChildren();
                shipParts[0].setVisible(false);
                shipParts[1].setVisible(false);
                shipParts[2].setVisible(false);
                shipParts[3].setStroke('#f00');
                var dots = shipParts[4].getChildren();
                dots[0].setFill('#f00');
                dots[1].setFill('#f00');
                dots[2].setFill('#f00');
                ship.setX(93);
                ship.setY(BerryBots.canvasHeight - 93);
                ship.setScale(3);
                BerryBots.stageLayer.add(ship);
                BerryBots.mainLayer.setOpacity(0.4);
              } else if (BerryBots.gameTime % 80 == 0) {
                var ship = BerryBots.spinnerShip;
                var dots = ship.getChildren()[4];
                dots.setRotation(dots.getRotation() + BerryBots.SPINNER_ROTATION);
                break;
              }
            } else {
              if (rs.nextShipState >= BerryBots.shipStates.length 
                  || BerryBots.gameTime >= BerryBots.nextDrawTime) {
                break;
              }
            }
          } while (true);
          BerryBots.nextDrawTime += BerryBots.playbackSpeed;
          if (BerryBots.skipTime != -1) {
            BerryBots.desktopPing();
            if (BerryBots.gameTime >= BerryBots.skipTime) {
              BerryBots.stopSkipping();
            }
          }
        } else if (!BerryBots.showedResults) {
          BerryBots.showResults();
          BerryBots.showedResults = true;
        }
      }
    }

    var stage = BerryBots.stage;
    var scale = Math.min(1, Math.min(window.innerWidth / stage.getWidth(),
                         window.innerHeight / stage.getHeight()));
    stage.setScale(scale, scale);
  }, BerryBots.layers);

  anim.start();
};


BerryBots.parseReplayData();
BerryBots.addBodyListeners();
BerryBots.replay();
