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

// This is a KineticJS hack from Elliot Chong's gist to force a maximum pixel
// ratio. Otherwise it runs horribly slow on high resolution devices.
// https://gist.github.com/ElliotChong/6107722

setMaximumPixelRatio = function(p_maximumRatio) {
  var backingStoreRatio, canvas, className, context, devicePixelRatio,
      pixelRatio, _i, _len, _ref, _results;

  if (p_maximumRatio == null) {
    p_maximumRatio = 1;
  }
  canvas = document.createElement('canvas');
  context = canvas.getContext('2d');
  devicePixelRatio = window.devicePixelRatio || 1;
  backingStoreRatio = context.webkitBackingStorePixelRatio
      || context.mozBackingStorePixelRatio || context.msBackingStorePixelRatio
      || context.oBackingStorePixelRatio || context.backingStorePixelRatio || 1;
  pixelRatio = devicePixelRatio / backingStoreRatio;
  _ref = ["HitCanvas", "SceneCanvas", "Canvas"];
  _results = [];
  for (_i = 0, _len = _ref.length; _i < _len; _i++) {
    className = _ref[_i];
    _results.push(Kinetic[className].prototype.init = (function(p_method) {
      return function(p_config) {
        if (p_config == null) {
          p_config = {};
        }
        if (p_config.pixelRatio != null) {
          pixelRatio = p_config.pixelRatio;
        }
        p_config.pixelRatio =
            pixelRatio > p_maximumRatio ? p_maximumRatio : pixelRatio;
        return p_method.call(this, p_config);
      };
    })(Kinetic[className].prototype.init));
  }
  return _results;
};

setMaximumPixelRatio(1);


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

BerryBots = {
  STAGE_MARGIN: 25,
  LASER_SPEED: 25,
  TORPEDO_SPEED: 12,
  SHIP_ROTATION: Math.PI * 2 / 240,
  THRUSTER_ZERO: 8.3 / 19.3,
  TWO_PI: 2 * Math.PI,
  ZONE_COLOR: '#644444',
  TORPEDO_COLOR: '#ff5926',
  LASER_SPARK_LENGTH: 8,
  LASER_SPARK_THICKNESS: 1.5,
  LASER_SPARK_TIME: 8,
  TORPEDO_RADIUS: 4,
  TORPEDO_BLAST_RADIUS: 100,
  TORPEDO_BLAST_TIME: 16,
  TORPEDO_DEBRIS_RADIUS: 1.5,
  TORPEDO_DEBRIS_TIME: 40,
  DESTROY_BASE_RADIUS: 6,
  DESTROY_CIRCLES: 3,
  DESTROY_FRAMES: 16,
  DESTROY_FRAME_LENGTH: 2,
  DESTROY_TIME: 32 /* 16 * 2 */,
  replayData: BerryBots.replayData
};

BerryBots.style = {};
BerryBots.style.consoleTabs =
      '<style type="text/css">'
    + '  .console-tab {'
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
    + '  .stage-tab { margin-bottom: 0.7em;}'
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
    + '  .play {'
    + '    display: block;'
    + '    height: 0;'
    + '    width: 0;'
    + '    border-bottom: 15px solid #fff;'
    + '    border-left: 15px solid transparent;'
    + '    transform: scale(6,3) rotate(-45deg);'
    + '    -ms-transform: scale(6,3) rotate(-45deg); '
    + '    -webkit-transform: scale(6,3) rotate(-45deg);'
    + '  }'
    + '  .play:hover {'
    + '    border-bottom-color: #0f0;'
    + '  }'
    + '  .pause-wedge {'
    + '    background-color: #fff;'
    + '    display: inline-block;'
    + '    height: 70px;'
    + '    width: 16px;'
    + '    margin-right: 8px;'
    + '  }'
    + '  .pause:hover .pause-wedge {'
    + '    background-color: #0f0;'
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
        + '    width: 24px;'
        + '    height: 24px;'
        + '    border-radius: 50%;'
        + '    position: relative;'
        + '  }'
        + '  .timetarget {'
        + '    height: 44px;'
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
  var d = new Date();
  BerryBots.hideOverlayTime =
      Math.max(BerryBots.hideOverlayTime, d.getTime() + 850);
}

BerryBots.touchPing = function() {
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
    fontSize: 14,
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
    width: BerryBots.LASER_SPARK_LENGTH,
    height: BerryBots.LASER_SPARK_THICKNESS,
    fill: '#00ff00',
    strokeWidth: 0,
    offset: [0, BerryBots.LASER_SPARK_THICKNESS / 2]
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
  return new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.TORPEDO_BLAST_RADIUS,
    stroke: BerryBots.TORPEDO_COLOR,
    strokeWidth: 5
  });
};

BerryBots.getTorpedoDebrisProto = function() {
  return new Kinetic.Circle({
    x: 0,
    y: 0,
    radius: BerryBots.TORPEDO_DEBRIS_RADIUS,
    strokeWidth: 0
  });
};

BerryBots.drawBackground = function(width, height) {
  var bgRect = new Kinetic.Rect({
    x: 0,
    y: 0,
    width: width + (BerryBots.STAGE_MARGIN * 2),
    height: height + (BerryBots.STAGE_MARGIN * 2),
    fill: 'black',
  });
  BerryBots.layer.add(bgRect);
};

BerryBots.getStage = function(width, height) {
  return new Kinetic.Stage({
    container: 'container',
    width: width + (BerryBots.STAGE_MARGIN * 2),
    height: height + (BerryBots.STAGE_MARGIN * 2)
  });
};

BerryBots.getStageProperties = function() {
  BerryBots.stageName = BerryBots.getString(1);
  BerryBots.stageWidth = BerryBots.getNumber(2);
  BerryBots.stageHeight = BerryBots.getNumber(3);
  BerryBots.stage =
      BerryBots.getStage(BerryBots.stageWidth, BerryBots.stageHeight);
};

BerryBots.drawRectangles = function(baseOffset, fillColor) {
  var numRectangles = BerryBots.getNumber(baseOffset);
  for (var x = 0; x < numRectangles; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var left = BerryBots.getNumber(offset);
    var bottom = BerryBots.getNumber(offset + 1);
    var width = BerryBots.getNumber(offset + 2);
    var height = BerryBots.getNumber(offset + 3);
    var wallRect = new Kinetic.Rect({
      x: BerryBots.STAGE_MARGIN + left,
      y: BerryBots.STAGE_MARGIN + BerryBots.stageHeight - height - bottom,
      width: width,
      height: height,
      fill: fillColor
    });
    BerryBots.layer.add(wallRect);
  }
  return numRectangles;
};

BerryBots.drawWallsAndZones = function() {
  var numWalls = BerryBots.drawRectangles(4, 'white');
  var zonesOffset = 5 + (numWalls * 4);
  var numZones = BerryBots.drawRectangles(zonesOffset, BerryBots.ZONE_COLOR);
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
    BerryBots.layer.add(ship);
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

// Parses the core / global replay data and draws it. This includes the stage
// itself, walls, zones, teams, and ships.
BerryBots.parseGlobalReplayData = function() {
  BerryBots.values = BerryBots.replayData.replace(/\\:/g, '@;@').split(':');
  BerryBots.getStageProperties();
  BerryBots.drawBackground(BerryBots.stageWidth, BerryBots.stageHeight);
  var offset = BerryBots.drawWallsAndZones();
  offset = BerryBots.parseTeamsAndShips(offset);
  BerryBots.stage.add(BerryBots.layer);
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
  var offset = BerryBots.parseGlobalReplayData();
  BerryBots.parseDynamicReplayData(offset);
};

// Initialize KineticJS graphics.
BerryBots.initGfx = function() {
  BerryBots.layer = new Kinetic.Layer();
  BerryBots.shipProto = BerryBots.getShipProto();
  BerryBots.shipDestroyProto = BerryBots.getShipDestroyProto();
  BerryBots.laserProto = BerryBots.getLaserProto();
  BerryBots.laserSparkProto = BerryBots.getLaserSparkProto();
  BerryBots.torpedoBlastProto = BerryBots.getTorpedoBlastProto();
  BerryBots.torpedoProto = BerryBots.getTorpedoProto();
  BerryBots.torpedoDebrisProto = BerryBots.getTorpedoDebrisProto();
};

BerryBots.addBodyListeners = function() {
  window.addEventListener('load', function() {
    document.body.onmousemove = function(e) {
      BerryBots.desktopPing();
      if (BerryBots.timeDragging) {
        BerryBots.mouseX = e.pageX;
        BerryBots.clearSelection();
      }
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

        if (BerryBots.resultsDiv != null) {
          BerryBots.hideResults();
        }
        BerryBots.timeDragging = false;
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

BerryBots.showConsole = function(teamIndex) {
  var numTeams = BerryBots.teams.length;
  for (var x = -1; x < numTeams; x++) {
    var console = BerryBots.getConsole(x);
    if (x != teamIndex && console.showing) {
      BerryBots.hideConsole(x);
    }
  }

  var console = BerryBots.getConsole(teamIndex);
  var consoleId = console.consoleId;
  if (console.div == null) {
    var s = '<style type="text/css">.console { border-collapse: collapse; '
        + 'border: 1px solid #fff; padding: 0.5em; background-color: #000; '
        + 'font-family: Consolas, Courier, monospace; font-size: 0.8em; '
        + 'color: #fff; overflow: auto; width: 550px; height: 400px; } '
        + '.console-title { background-color: #fff; color: #000; '
        + 'position: relative; text-align: center; padding: 5px; '
        + 'font-family: Ubuntu, Arial, Tahoma, sans-serif; }'
        + '.console-x { font-size: 1.5em; position: absolute; left: 6px; '
        + 'cursor: pointer; top: 0; } .console-x:hover { color: #f00; }</style>'
        + '<div class="console-title"><div class="console-x" '
        + 'onclick="BerryBots.hideConsole(' + teamIndex + ')">&times;</div>'
        + console.name + '</div><div class="console" id="' + consoleId + '">';

    var messages = console.logMessages;
    var numMessages = messages.length;
    for (var x = 0; x < numMessages; x++) {
      messages[x] = BerryBots.escapeHtml(messages[x]);
    }
    s += console.logMessages.join('<br>');
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
  d.style.left = Math.max(0, ((BerryBots.stage.getScaleX()
      * BerryBots.stage.getWidth()) - 600)) + 'px';
  d.style.top = '35px';

  console.showing = true;
  BerryBots.scrollToBottom(console.outputDiv);
};

BerryBots.hideResults = function() {
  if (BerryBots.resultsDiv != null) {
    document.getElementById('container').removeChild(BerryBots.resultsDiv);
  }
};

BerryBots.playPause = function() {
  BerryBots.paused = !(BerryBots.paused);
  BerryBots.showPlayPause();
};

BerryBots.showPlayPause = function() {
  var stage = BerryBots.stage;
  var center =
      Math.max(0, (stage.getScaleX() * stage.getWidth()) / 2);
  var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 150);

  var d = document.getElementById('controls');
  d.style.position = 'absolute';
  if (BerryBots.paused) {
    d.style.left = (center - 33) + 'px';
    d.style.top = top + 'px';
    document.getElementById('pause').style.visibility = 'hidden';
    document.getElementById('play').style.visibility = 'visible';
  } else {
    d.style.left = (center - 20) + 'px';
    d.style.top = (top - 42) + 'px';
    document.getElementById('play').style.visibility = 'hidden';
    document.getElementById('pause').style.visibility = 'visible';
  }
};

BerryBots.showOverlay = function() {
  BerryBots.showConsoleTabs();
  BerryBots.showControls();
  BerryBots.showTimeDisplay();
  BerryBots.showingOverlay = true;
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
      + 'onclick="BerryBots.showConsole(-1)">' + BerryBots.stageName + '</div>'

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
      s += '<div class="console-tab" onclick="BerryBots.showConsole('
          + teams[x].index + ')">' + teams[x].name + '</div>';
    }
  }

  var d = document.createElement('div');
  d.innerHTML = s;
  d.id = 'dock';
  d.style.color = '#fff';
  d.margin = '0';
  d.padding = '0';
  d.style.fontFamily = 'Ubuntu, Arial, Tahoma, sans-serif';
  d.style.fontSize = '1.12em';
  d.style.position = 'absolute';
  d.style.left = '35px';
  d.style.top = '35px';
  document.getElementById('container').appendChild(d);
};

BerryBots.showControls = function() {
  var s = BerryBots.style.controls
      + '<div class="controls" onclick="BerryBots.playPause()">'
      + '  <div id="play" class="play"></div>'
      + '  <div id="pause" class="pause">'
      + '    <div class="pause-wedge"></div>'
      + '    <div class="pause-wedge"></div>'
      + '  </div>'
      + '</div>';

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
  var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 85);
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
  timeTarget.onmouseup = function(e) {
    BerryBots.desktopPing();
    BerryBots.timeMouseUp(e);
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
  knobStyle.top = '6px';

  var stage = BerryBots.stage;
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var timerWidth = stageWidth / 2;
  if (BerryBots.timeDragging) {
    var left = stageWidth / 4;
    // Visual feedback you can't rewind (for now).
    var minX = (BerryBots.gameTime / BerryBots.endTime) * timerWidth;
    knobStyle.left = (Math.max(minX,
        Math.min(timerWidth, (BerryBots.mouseX - left))) + 8) + 'px';
  } else if (BerryBots.gameTime >= BerryBots.skipTime) {
    var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 75);

    var left = (BerryBots.gameTime / BerryBots.endTime) * (stageWidth / 2) + 8;
    knobStyle.left = left + 'px';
  }
};

BerryBots.timeMouseDown = function(e) {
  BerryBots.timeDragging = true;
  BerryBots.mouseX = e.pageX;
  BerryBots.adjustTimeKnob();
};

BerryBots.timeMouseUp = function(e) {
  BerryBots.dragEnd(e.pageX);
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
  var s = '<style type="text/css">table, td, th { border-collapse: collapse; '
      + 'background-color: #fff; border: 1px solid #000; color: #000 }'
      + '.num { text-align: right; } .mid { text-align: center; }'
      + '.results-x { font-size: 1.8em; position: absolute; left: 8px; '
      + 'cursor: pointer; top: 0; } .results-x:hover { color: #f00; }'
      + '.rel { position: relative; }'
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
    s += '<table id="resultsTable" cellpadding="9px">'
        + '<tr><td colspan="' + numCols + '" class="mid rel">'
        + '<div class="results-x" onclick="BerryBots.hideResults()">&times;'
        + '</div>Results</td></tr><tr><td class="mid">Rank</td>'
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
  s += '</table>\n';

  var d = document.createElement('div');
  d.innerHTML = s;
  d.margin = '0';
  d.padding = '0';
  d.style.fontFamily = 'Ubuntu, Arial, Tahoma, sans-serif';
  d.style.fontSize = '1em';
  document.getElementById('container').appendChild(d);

  var resultsTable = document.getElementById('resultsTable');
  var stage = BerryBots.stage;
  var left = Math.max(0,
      ((stage.getScaleX() * stage.getWidth()) - resultsTable.clientWidth) / 2);
  var top = Math.max(0,
      ((stage.getScaleY() * stage.getHeight())
          - resultsTable.clientHeight) / 2);
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
    var shipX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset) / 10;
    var shipY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 1) / 10);
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
    var srcX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 3) / 10;
    var srcY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 4) / 10);
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
    var sparkX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10;
    var sparkY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10);
    var dx = BerryBots.getNumber(offset + 4) / 100;
    var dy = BerryBots.getNumber(offset + 5) / 100;
    laserSparks[x] = {shipIndex: shipIndex, startTime: time, x: sparkX,
        y: sparkY, dx: dx, dy: dy, endTime: time + BerryBots.LASER_SPARK_TIME};
  }
  return laserSparks;
}

BerryBots.getTorpedoBlasts = function(baseOffset) {
  var numTorpedoBlasts = BerryBots.getNumber(baseOffset);
  var torpedoBlasts = new Array();
  for (var x = 0; x < numTorpedoBlasts; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var time = BerryBots.getNumber(offset);
    var blastX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 1) / 10;
    var blastY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 2) / 10);
    torpedoBlasts[x] = {startTime: time, x: blastX, y: blastY,
                        endTime: time + BerryBots.TORPEDO_BLAST_TIME};
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
    var debrisX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10;
    var debrisY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10);
    var dx = BerryBots.getNumber(offset + 4) / 100;
    var dy = BerryBots.getNumber(offset + 5) / 100;
    var parts = BerryBots.getNumber(offset + 6);
    torpedoDebris[x] = {shipIndex: shipIndex, startTime: time, x: debrisX,
        y: debrisY, dx: dx, dy: dy, parts: parts,
        speed: ((Math.random() + 1) / 2) * 2.5,
        endTime: time + BerryBots.TORPEDO_DEBRIS_TIME};
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
    var shipX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10;
    var shipY = BerryBots.STAGE_MARGIN
        + BerryBots.stageHeight - (BerryBots.getNumber(offset + 3) / 10);
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
    var size = BerryBots.getNumber(offset + 4);
    var textX = BerryBots.STAGE_MARGIN + BerryBots.getNumber(offset + 2) / 10;
    var textY = BerryBots.STAGE_MARGIN + BerryBots.stageHeight
        - (BerryBots.getNumber(offset + 3) / 10) - size;
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
          .replace(/'/g, "&#039;");
};

// Replay the match from our data model, tick by tick.
BerryBots.replay = function() {
  var ships = BerryBots.ships;
  var numShips = BerryBots.ships.length;

  var shipAlives = new Array();
  for (var x = 0; x < numShips; x++) {
    shipAlives[x] = false;
  }

  var shipShowNames = BerryBots.shipShowNames = new Array();
  for (var x = 0; x < numShips; x++) {
    shipShowNames[x] = false;
  }

  var shipShowEnergys = new Array();
  for (var x = 0; x < numShips; x++) {
    shipShowEnergys[x] = false;
  }

  BerryBots.gameTime = 0;
  BerryBots.skipTime = -1;
  var nextShipState = 0;
  var nextShipAdd = 0;
  var nextShipRemove = 0;
  var nextShipShowName = 0;
  var nextShipHideName = 0;
  var nextShipShowEnergy = 0;
  var nextShipHideEnergy = 0;
  var shipDestroyCircles = new Array();
  var nextShipDestroy = 0;

  var lasers = new Array();
  var laserSparkRects = new Array();
  var nextLaserStart = 0;
  var nextLaserEnd = 0;
  var nextLaserSpark = 0;

  var torpedos = new Array();
  var torpedoBlastCircles = new Array();
  var torpedoDebrisCircles = new Array();
  var nextTorpedoStart = 0;
  var nextTorpedoEnd = 0;
  var nextTorpedoBlast = 0;
  var nextTorpedoDebris = 0;

  var stageTextShapes = new Array();
  var nextStageText = 0;
  BerryBots.stageConsole = new Object();
  BerryBots.stageConsole.logMessages = new Array();
  BerryBots.stageConsole.div = null;
  BerryBots.stageConsole.outputDiv = null;
  BerryBots.stageConsole.showing = false;
  BerryBots.stageConsole.consoleId = 'console0';
  BerryBots.stageConsole.name = BerryBots.stageName;
  var nextLogEntry = 0;
  BerryBots.knob = null;

  BerryBots.showedResults = false;
  BerryBots.resultsDiv = null;

  BerryBots.hideOverlayTime = 0;
  BerryBots.showingOverlay = false;
  BerryBots.paused = false;
  BerryBots.timeDragging = false;

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
      if (nextShipState < BerryBots.shipStates.length) {
        do {
          var gameTime = BerryBots.gameTime;

          // Lasers.
          var laserStarts = BerryBots.laserStarts;
          var numLaserStarts = laserStarts.length;
          while (nextLaserStart < numLaserStarts
                 && laserStarts[nextLaserStart].fireTime <= gameTime) {
            var laserStart = laserStarts[nextLaserStart++];
            var laser = BerryBots.laserProto.clone();
            var dx = Math.cos(laserStart.heading) * BerryBots.LASER_SPEED;
            var dy = Math.sin(laserStart.heading) * BerryBots.LASER_SPEED;
            laser.setX(laserStart.srcX);
            laser.setY(laserStart.srcY);
            laser.setFill(ships[laserStart.shipIndex].laserColor);
            laser.setRotation(BerryBots.TWO_PI - laserStart.heading);
            laser.laserData = {dx: dx, dy: dy, id: laserStart.id};
            lasers.push(laser);
            BerryBots.layer.add(laser);
          }

          var laserEnds = BerryBots.laserEnds;
          var numLaserEnds = laserEnds.length;
          while (nextLaserEnd < numLaserEnds
                 && laserEnds[nextLaserEnd].time <= gameTime) {
            var laserEnd = laserEnds[nextLaserEnd++];
            // TODO: use a map instead?
            for (var x = 0; x < lasers.length; x++) {
              var laser = lasers[x];
              if (laser.laserData.id == laserEnd.id) {
                lasers.splice(x--, 1);
                laser.destroy();
              }
            }
          }

          for (var x = 0; x < lasers.length; x++) {
            var laser = lasers[x];
            laser.setX(laser.getX() + laser.laserData.dx);
            laser.setY(laser.getY() - laser.laserData.dy);
          }

          for (var x = 0; x < laserSparkRects.length; x++) {
            var laserSparkRect = laserSparkRects[x];
            if (laserSparkRect.sparkData.endTime < gameTime) {
              laserSparkRects.splice(x--, 1);
              laserSparkRect.destroy();
            } else {
              laserSparkRect.setOffset(
                  {x: laserSparkRect.getOffset().x + 5,
                   y: BerryBots.LASER_SPARK_THICKNESS / 2});
              laserSparkRect.move(laserSparkRect.sparkData.dx,
                                  -laserSparkRect.sparkData.dy);
            }
          }

          var laserSparks = BerryBots.laserSparks;
          var numLaserSparks = laserSparks.length;
          while (nextLaserSpark < numLaserSparks
                 && laserSparks[nextLaserSpark].startTime <= gameTime) {
            var laserSpark = laserSparks[nextLaserSpark++];
            for (var x = 0; x < 4; x++) {
              var laserSparkRect = BerryBots.laserSparkProto.clone();
              laserSparkRect.setX(laserSpark.x);
              laserSparkRect.setY(laserSpark.y);
              laserSparkRect.setFill(ships[laserSpark.shipIndex].laserColor);
              laserSparkRect.setRotation(Math.random() * BerryBots.TWO_PI);
              laserSparkRect.setOffset(
                  {x: 9, y: BerryBots.LASER_SPARK_THICKNESS / 2});
              laserSparkRect.sparkData = laserSpark;
              laserSparkRects.push(laserSparkRect);
              BerryBots.layer.add(laserSparkRect);
            }
          }

          // Torpedos.
          var torpedoStarts = BerryBots.torpedoStarts;
          var numTorpedoStarts = torpedoStarts.length;
          while (nextTorpedoStart < numTorpedoStarts
                 && torpedoStarts[nextTorpedoStart].fireTime <= gameTime) {
            var torpedoStart = torpedoStarts[nextTorpedoStart++];
            var torpedo = BerryBots.torpedoProto.clone();
            var dx = Math.cos(torpedoStart.heading) * BerryBots.TORPEDO_SPEED;
            var dy = Math.sin(torpedoStart.heading) * BerryBots.TORPEDO_SPEED;
            torpedo.setX(torpedoStart.srcX);
            torpedo.setY(torpedoStart.srcY);
            torpedo.torpedoData = {dx: dx, dy: dy, id: torpedoStart.id};
            torpedos.push(torpedo);
            BerryBots.layer.add(torpedo);
          }

          var torpedoEnds = BerryBots.torpedoEnds;
          var numTorpedoEnds = torpedoEnds.length;
          while (nextTorpedoEnd < numTorpedoEnds
                 && torpedoEnds[nextTorpedoEnd].time <= gameTime) {
            var torpedoEnd = torpedoEnds[nextTorpedoEnd++];
            // TODO: use a map instead?
            for (var x = 0; x < torpedos.length; x++) {
              var torpedo = torpedos[x];
              if (torpedo.torpedoData.id == torpedoEnd.id) {
                torpedos.splice(x--, 1);
                torpedo.destroy();
              }
            }
          }

          for (var x = 0; x < torpedos.length; x++) {
            var torpedo = torpedos[x];
            torpedo.setX(torpedo.getX() + torpedo.torpedoData.dx);
            torpedo.setY(torpedo.getY() - torpedo.torpedoData.dy);
          }

          for (var x = 0; x < torpedoBlastCircles.length; x++) {
            var torpedoBlastCircle = torpedoBlastCircles[x];
            if (torpedoBlastCircle.blastData.endTime < gameTime) {
              torpedoBlastCircles.splice(x--, 1);
              torpedoBlastCircle.destroy();
            } else {
              var blastTime = gameTime - torpedoBlastCircle.blastData.startTime;
              var blastScale;
              if (blastTime < 5) {
                blastScale = (4 - blastTime) / 4;
              } else {
                blastScale =
                    (blastTime - 3) / (BerryBots.TORPEDO_BLAST_TIME - 4);
              }
              torpedoBlastCircle.setScale(blastScale);
            }
          }

          var torpedoBlasts = BerryBots.torpedoBlasts;
          var numTorpedoBlasts = torpedoBlasts.length;
          while (nextTorpedoBlast < numTorpedoBlasts
                 && torpedoBlasts[nextTorpedoBlast].startTime <= gameTime) {
            var torpedoBlast = torpedoBlasts[nextTorpedoBlast++];
            var torpedoBlastCircle = BerryBots.torpedoBlastProto.clone();
            torpedoBlastCircle.setX(torpedoBlast.x);
            torpedoBlastCircle.setY(torpedoBlast.y);
            torpedoBlastCircle.blastData = torpedoBlast;
            torpedoBlastCircles.push(torpedoBlastCircle);
            BerryBots.layer.add(torpedoBlastCircle);
          }

          for (var x = 0; x < torpedoDebrisCircles.length; x++) {
            var torpedoDebrisCircle = torpedoDebrisCircles[x];
            if (torpedoDebrisCircle.debrisData.endTime < gameTime) {
              torpedoDebrisCircles.splice(x--, 1);
              torpedoDebrisCircle.destroy();
            } else {
              torpedoDebrisCircle.setOffset(
                  {x: torpedoDebrisCircle.getOffset().x
                      + torpedoDebrisCircle.debrisData.speed});
              torpedoDebrisCircle.move(torpedoDebrisCircle.debrisData.dx,
                                      -torpedoDebrisCircle.debrisData.dy);
            }
          }

          var torpedoDebris = BerryBots.torpedoDebris;
          var numTorpedoDebris = torpedoDebris.length;
          while (nextTorpedoDebris < numTorpedoDebris
                 && torpedoDebris[nextTorpedoDebris].startTime <= gameTime) {
            var debrisData = torpedoDebris[nextTorpedoDebris++];
            for (var x = 0; x < debrisData.parts; x++) {
              var torpedoDebrisCircle = BerryBots.torpedoDebrisProto.clone();
              torpedoDebrisCircle.setX(debrisData.x);
              torpedoDebrisCircle.setY(debrisData.y);
              torpedoDebrisCircle.setFill(
                  ships[debrisData.shipIndex].shipColor);
              torpedoDebrisCircle.setRotation(Math.random() * BerryBots.TWO_PI);
              torpedoDebrisCircle.setOffset({x: 9});
              torpedoDebrisCircle.debrisData = debrisData;
              torpedoDebrisCircles.push(torpedoDebrisCircle);
              BerryBots.layer.add(torpedoDebrisCircle);
            }
          }

          // Ship destroys.
          for (var x = 0; x < shipDestroyCircles.length; x++) {
            var shipDestroyCircle = shipDestroyCircles[x];
            if (shipDestroyCircle.destroyData.endTime < gameTime) {
              shipDestroyCircles.splice(x--, 1);
              shipDestroyCircle.destroy();
            } else {
              var radiusFactor = 1 + Math.floor(
                  (gameTime - shipDestroyCircle.destroyData.startTime) / 2);
              shipDestroyCircle.setRadius(
                  radiusFactor * BerryBots.DESTROY_BASE_RADIUS);
            }
          }

          var shipDestroys = BerryBots.shipDestroys;
          var numShipDestroys = shipDestroys.length;
          while (nextShipDestroy < numShipDestroys
                 && shipDestroys[nextShipDestroy].startTime <= gameTime) {
            var shipDestroy = shipDestroys[nextShipDestroy++];
            var shipDestroyCircle = BerryBots.shipDestroyProto.clone();
            shipDestroyCircle.setX(shipDestroy.x);
            shipDestroyCircle.setY(shipDestroy.y);
            shipDestroyCircle.setStroke(ships[shipDestroy.shipIndex].shipColor);
            shipDestroyCircle.destroyData = shipDestroy;
            shipDestroyCircles.push(shipDestroyCircle);
            BerryBots.layer.add(shipDestroyCircle);
          }

          // Ships.
          var shipAdds = BerryBots.shipAdds;
          var numShipAdds = shipAdds.length;
          while (nextShipAdd < numShipAdds
                 && shipAdds[nextShipAdd].time <= gameTime) {
            shipAlives[shipAdds[nextShipAdd++].index] = true;
          }

          var shipRemoves = BerryBots.shipRemoves;
          var numShipRemoves = shipRemoves.length;
          while (nextShipRemove < numShipRemoves
                 && shipRemoves[nextShipRemove].time <= gameTime) {
            shipAlives[shipRemoves[nextShipRemove++].index] = false;
          }

          var shipAddShowNames = BerryBots.shipAddShowNames;
          var numShipAddShowNames = shipAddShowNames.length;
          while (nextShipShowName < numShipAddShowNames
                 && shipAddShowNames[nextShipShowName].time <= gameTime) {
            shipShowNames[shipAddShowNames[nextShipShowName++].index] = true;
          }

          var shipAddHideNames = BerryBots.shipAddHideNames;
          var numShipAddHideNames = shipAddHideNames.length;
          while (nextShipHideName < numShipAddHideNames
                 && shipAddHideNames[nextShipHideName].time <= gameTime) {
            shipShowNames[shipAddHideNames[nextShipHideName++].index] = false;
          }

          var shipAddShowEnergys = BerryBots.shipAddShowEnergys;
          var numShipAddShowEnergys = shipAddShowEnergys.length;
          while (nextShipShowEnergy < numShipAddShowEnergys
                 && shipAddShowEnergys[nextShipShowEnergy].time <= gameTime) {
            var shipIndex = shipAddShowEnergys[nextShipShowEnergy++].index;
            shipShowEnergys[shipIndex] = true;
            ships[shipIndex].getChildren()[1].setPosition(0, 24);
          }

          var shipAddHideEnergys = BerryBots.shipAddHideEnergys;
          var numShipAddHideEnergys = shipAddHideEnergys.length;
          while (nextShipHideEnergy < numShipAddHideEnergys
                 && shipAddHideEnergys[nextShipHideEnergy].time <= gameTime) {
            var shipIndex = shipAddHideEnergys[nextShipHideEnergy++].index;
            shipShowEnergys[shipIndex] = false;
            ships[shipIndex].getChildren()[1].setPosition(0, 16);
          }

          for (var x = 0; x < numShips; x++) {
            if (shipAlives[x]) {
              var shipState = BerryBots.shipStates[nextShipState++];
              ships[x].show();
              ships[x].setX(shipState.x);
              ships[x].setY(shipState.y);
              var thruster = ships[x].getChildren()[0];
              thruster.setRotation(
                  Math.PI + (BerryBots.TWO_PI - shipState.thrusterAngle));
              var forceFactor = shipState.thrusterForce;
              if (forceFactor < 0.0001) {
                thruster.setScale(0);
              } else {
                thruster.setScale(BerryBots.THRUSTER_ZERO
                    + (forceFactor * (1 - BerryBots.THRUSTER_ZERO)));
              }

              ships[x].getChildren()[2].setScale(shipState.energy / 100, 1);
              var shipDotGroup = ships[x].getChildren()[4];
              shipDotGroup.setRotation(shipDotGroup.getRotation()
                  + (BerryBots.SHIP_ROTATION * BerryBots.shipOrbits[x]));
              ships[x].getChildren()[1].setVisible(shipShowNames[x]);
              ships[x].getChildren()[2].setVisible(shipShowEnergys[x]);
            } else {
              ships[x].hide();
            }
          }

          // Stage texts.
          for (var x = 0; x < stageTextShapes.length; x++) {
            var stageTextShape = stageTextShapes[x];
            if (stageTextShape.textData.endTime < gameTime) {
              stageTextShapes.splice(x--, 1);
              stageTextShape.destroy();
            }
          }

          var stageTexts = BerryBots.stageTexts;
          var numStageTexts = stageTexts.length;
          while (nextStageText < numStageTexts
                 && stageTexts[nextStageText].startTime <= gameTime) {
            var stageText = stageTexts[nextStageText++];
            var stageTextShape = new Kinetic.Text({
              x: stageText.x,
              y: stageText.y,
              text: stageText.text,
              fontSize: stageText.size,
              fontFamily: 'Questrial, Ubuntu, Arial, Tahoma, sans-serif',
              fill: stageText.color,
              strokeWidth: 0
            });
            stageTextShape.textData = stageText;
            stageTextShapes.push(stageTextShape);
            BerryBots.layer.add(stageTextShape);
          }

          var logEntries = BerryBots.logEntries;
          var numLogEntries = logEntries.length;
          while (nextLogEntry < numLogEntries
                 && logEntries[nextLogEntry].time <= gameTime) {
            var logEntry = logEntries[nextLogEntry++];
            var console = BerryBots.getConsole(logEntry.teamIndex);

            console.logMessages.push(logEntry.message);
            if (console.div != null) {
              console.outputDiv.innerHTML +=
                  '<br>' + BerryBots.escapeHtml(logEntry.message);
              BerryBots.scrollToBottom(console.outputDiv);
            }
          }

          BerryBots.gameTime++;
        } while (BerryBots.gameTime < BerryBots.skipTime);
        if (BerryBots.skipTime != -1) {
          BerryBots.desktopPing();
        }
        BerryBots.skipTime = -1;
      } else if (!BerryBots.showedResults) {
        BerryBots.showResults();
        BerryBots.showedResults = true;
      }
    }

    var stage = BerryBots.stage;
    var scale = Math.min(window.innerWidth / stage.getWidth(),
                         window.innerHeight / stage.getHeight());
    stage.setScale(Math.min(1, scale, scale));
  }, BerryBots.layer);

  anim.start();
};


BerryBots.initGfx();
BerryBots.parseReplayData();
BerryBots.addBodyListeners();
BerryBots.replay();
