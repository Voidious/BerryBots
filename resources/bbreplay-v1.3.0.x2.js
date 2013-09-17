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

var BerryBots = {
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
};

// Define some KineticJS graphics objects.

var layer = new Kinetic.Layer();

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

var shipDestroyProto = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: BerryBots.DESTROY_BASE_RADIUS,
  strokeWidth: 2
});

var laserProto = new Kinetic.Rect({
  x: 0,
  y: 0,
  width: 25,
  height: 2,
  fill: '#00ff00',
  strokeWidth: 0,
  offset: [0, 1]
});

var laserSparkProto = new Kinetic.Rect({
  x: 0,
  y: 0,
  width: BerryBots.LASER_SPARK_LENGTH,
  height: BerryBots.LASER_SPARK_THICKNESS,
  fill: '#00ff00',
  strokeWidth: 0,
  offset: [0, BerryBots.LASER_SPARK_THICKNESS / 2]
});

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

var torpedoBlastProto = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: BerryBots.TORPEDO_BLAST_RADIUS,
  stroke: BerryBots.TORPEDO_COLOR,
  strokeWidth: 5
});

var torpedoDebrisProto = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: BerryBots.TORPEDO_DEBRIS_RADIUS,
  strokeWidth: 0
});

var torpedoRay1 = torpedoRay.clone();
torpedoRay1.setRotation(Math.PI / 4);
var torpedoRay2 = torpedoRay.clone();
torpedoRay2.setRotation(-Math.PI / 4);

var torpedoProto = new Kinetic.Group({x: 0, y: 0});
torpedoProto.add(torpedoCircle);
torpedoProto.add(torpedoRay1);
torpedoProto.add(torpedoRay2);

var shipName = new Kinetic.Text({
  x: 0,
  y: 16,
  text: '',
  fontSize: 14,
  fontFamily: 'Ubuntu, Arial, Tahoma, sans-serif',
  fill: '#e0e0e0'
});

var shipDotGroup = new Kinetic.Group({x: 0, y: 0});
shipDotGroup.add(shipDot1);
shipDotGroup.add(shipDot2);
shipDotGroup.add(shipDot3);

var shipGroup = new Kinetic.Group({x: 0, y: 0});
shipGroup.add(shipThruster);
shipGroup.add(shipName);
shipGroup.add(shipEnergy);
shipGroup.add(shipCircle);
shipGroup.add(shipDotGroup);


// Parse global stage stuff and draw it once.

var values = replayData.replace(/\\:/g, '@;@').split(':');
var stageName = getString(1);
var stageWidth = getNumber(2);
var stageHeight = getNumber(3);

var bgRect = new Kinetic.Rect({
  x: 0,
  y: 0,
  width: stageWidth + (BerryBots.STAGE_MARGIN * 2),
  height: stageHeight + (BerryBots.STAGE_MARGIN * 2),
  fill: 'black',
});
layer.add(bgRect);

var stage = new Kinetic.Stage({
  container: 'container',
  width: stageWidth + (BerryBots.STAGE_MARGIN * 2),
  height: stageHeight + (BerryBots.STAGE_MARGIN * 2)
});

var numWalls = drawRectangles(4, 'white');
var zonesOffset = 5 + (numWalls * 4);
var numZones = drawRectangles(zonesOffset, BerryBots.ZONE_COLOR);

var teamsOffset = zonesOffset + 1 + (numZones * 4);
var teams = getTeamProperties(teamsOffset);
var numTeams = teams.length;

var shipsOffset = teamsOffset + 1 + (numTeams * 2);
var ships = getShipProperties(shipsOffset);
var numShips = ships.length;
var shipOrbits = getShipOrbits(numShips);

stage.add(layer);


// Parse dynamic replay data for the whole match into a workable data model.

var shipAddsOffset = shipsOffset + 1 + (numShips * 5);
var shipAdds = getShipStateChanges(shipAddsOffset);
var numShipAdds = shipAdds.length;

var shipRemovesOffset = shipAddsOffset + 1 + (numShipAdds * 2);
var shipRemoves = getShipStateChanges(shipRemovesOffset);
var numShipRemoves = shipRemoves.length;

var shipAddShowNamesOffset = shipRemovesOffset + 1 + (numShipRemoves * 2);
var shipAddShowNames = getShipStateChanges(shipAddShowNamesOffset);
var numShipAddShowNames = shipAddShowNames.length;

var shipAddHideNamesOffset =
    shipAddShowNamesOffset + 1 + (numShipAddShowNames * 2);
var shipAddHideNames = getShipStateChanges(shipAddHideNamesOffset);
var numShipAddHideNames = shipAddHideNames.length;

var shipAddShowEnergysOffset =
    shipAddHideNamesOffset + 1 + (numShipAddHideNames * 2);
var shipAddShowEnergys = getShipStateChanges(shipAddShowEnergysOffset);
var numShipAddShowEnergys = shipAddShowEnergys.length;

var shipAddHideEnergysOffset =
    shipAddShowEnergysOffset + 1 + (numShipAddShowEnergys * 2);
var shipAddHideEnergys = getShipStateChanges(shipAddHideEnergysOffset);
var numShipAddHideEnergys = shipAddHideEnergys.length;

var shipStatesOffset =
    shipAddHideEnergysOffset + 1 + (numShipAddHideEnergys * 2);
var shipStates = getShipStates(shipStatesOffset);
var numShipStates = shipStates.length;

var laserStartsOffset = shipStatesOffset + 1 + (numShipStates * 5);
var laserStarts = getProjectileStarts(laserStartsOffset);
var numLaserStarts = laserStarts.length;

var laserEndsOffset = laserStartsOffset + 1 + (numLaserStarts * 6);
var laserEnds = getProjectileEnds(laserEndsOffset);
var numLaserEnds = laserEnds.length;

var laserSparksOffset = laserEndsOffset + 1 + (numLaserEnds * 2);
var laserSparks = getLaserSparks(laserSparksOffset);
var numLaserSparks = laserSparks.length;

var torpedoStartsOffset = laserSparksOffset + 1 + (numLaserSparks * 6);
var torpedoStarts = getProjectileStarts(torpedoStartsOffset);
var numTorpedoStarts = torpedoStarts.length;

var torpedoEndsOffset = torpedoStartsOffset + 1 + (numTorpedoStarts * 6);
var torpedoEnds = getProjectileEnds(torpedoEndsOffset);
var numTorpedoEnds = torpedoEnds.length;

var torpedoBlastsOffset = torpedoEndsOffset + 1 + (numTorpedoEnds * 2);
var torpedoBlasts = getTorpedoBlasts(torpedoBlastsOffset);
var numTorpedoBlasts = torpedoBlasts.length;

var torpedoDebrisOffset = torpedoBlastsOffset + 1 + (numTorpedoBlasts * 3);
var torpedoDebris = getTorpedoDebris(torpedoDebrisOffset);
var numTorpedoDebris = torpedoDebris.length;

var shipDestroysOffset = torpedoDebrisOffset + 1 + (numTorpedoDebris * 7);
var shipDestroys = getShipDestroys(shipDestroysOffset);
var numShipDestroys = shipDestroys.length;

var stageTextsOffset = shipDestroysOffset + 1
    + ((numShipDestroys / BerryBots.DESTROY_CIRCLES) * 4);
var stageTexts = getStageTexts(stageTextsOffset);
var numStageTexts = stageTexts.length;

var logEntriesOffset = stageTextsOffset + 1 + (numStageTexts * 8);
var logEntries = getLogEntries(logEntriesOffset);
var numLogEntries = logEntries.length;

var resultsOffset = logEntriesOffset + 1 + (numLogEntries * 3);
var results = getResults(resultsOffset);
var numResults = results.length;


// Replay the match from our data model, tick by tick.

var shipAlives = new Array();
for (var x = 0; x < numShips; x++) {
  shipAlives[x] = false;
}

var shipShowNames = new Array();
for (var x = 0; x < numShips; x++) {
  shipShowNames[x] = false;
}

var shipShowEnergys = new Array();
for (var x = 0; x < numShips; x++) {
  shipShowEnergys[x] = false;
}

var gameTime = 0;
var skipTime = -1;
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
var stageConsole = new Object();
stageConsole.logMessages = new Array();
stageConsole.div = null;
stageConsole.outputDiv = null;
stageConsole.showing = false;
stageConsole.consoleId = 'console0';
stageConsole.name = stageName;
var nextLogEntry = 0;
var knob = null;

var showedResults = false;
var resultsDiv = null;

var lastMove = 0;
var lastHover = 0;
var lastMouseOut = 0;
var startShowing = 0;
var showingOverlay = false;
var paused = false;
var timeDragging = false;
var endTime = getEndTime(numShips, shipStates, shipAdds, shipRemoves);
var mouseX = 0;

function overlayPing() {
  var d = new Date();
  lastMove = d.getTime();
}

document.getElementsByTagName('body')[0].onmousemove = function(e) {
  overlayPing();
  if (timeDragging) {
    mouseX = e.pageX;
  }
};

document.getElementsByTagName('body')[0].onkeydown = function(e) {
  if (e.which == 27) { // escape
    for (var x = -1; x < numTeams; x++) {
      var console = getConsole(x);
      if (console.showing) {
        hideConsole(x);
      }
    }

    if (resultsDiv != null) {
      hideResults();
    }
  } else if (e.which == 32) { // space bar
    overlayPing();
    playPause();
  }
};

var anim = new Kinetic.Animation(function(frame) {
  var d = new Date();
  var now = d.getTime();
  if (now - lastMove < 850) {
    if (!showingOverlay) {
      showOverlay();
    }
  } else {
    if (showingOverlay) {
      hideOverlay();
    }
  }

  if (showingOverlay) {
    adjustTimeKnob();
  }
  if (!paused) {
    if (nextShipState < numShipStates) {
      do {
        // Lasers.
        while (nextLaserStart < numLaserStarts
               && laserStarts[nextLaserStart].fireTime <= gameTime) {
          var laserStart = laserStarts[nextLaserStart++];
          var laser = laserProto.clone();
          var dx = Math.cos(laserStart.heading) * BerryBots.LASER_SPEED;
          var dy = Math.sin(laserStart.heading) * BerryBots.LASER_SPEED;
          laser.setX(laserStart.srcX);
          laser.setY(laserStart.srcY);
          laser.setFill(ships[laserStart.shipIndex].laserColor);
          laser.setRotation(BerryBots.TWO_PI - laserStart.heading);
          laser.laserData = {dx: dx, dy: dy, id: laserStart.id};
          lasers.push(laser);
          layer.add(laser);
        }

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
            laserSparkRect.setOffset({x: laserSparkRect.getOffset().x + 5,
                                      y: BerryBots.LASER_SPARK_THICKNESS / 2});
            laserSparkRect.move(laserSparkRect.sparkData.dx,
                                -laserSparkRect.sparkData.dy);
          }
        }

        while (nextLaserSpark < numLaserSparks
               && laserSparks[nextLaserSpark].startTime <= gameTime) {
          var laserSpark = laserSparks[nextLaserSpark++];
          for (var x = 0; x < 4; x++) {
            var laserSparkRect = laserSparkProto.clone();
            laserSparkRect.setX(laserSpark.x);
            laserSparkRect.setY(laserSpark.y);
            laserSparkRect.setFill(ships[laserSpark.shipIndex].laserColor);
            laserSparkRect.setRotation(Math.random() * BerryBots.TWO_PI);
            laserSparkRect.setOffset(
                {x: 9, y: BerryBots.LASER_SPARK_THICKNESS / 2});
            laserSparkRect.sparkData = laserSpark;
            laserSparkRects.push(laserSparkRect);
            layer.add(laserSparkRect);
          }
        }

        // Torpedos.
        while (nextTorpedoStart < numTorpedoStarts
               && torpedoStarts[nextTorpedoStart].fireTime <= gameTime) {
          var torpedoStart = torpedoStarts[nextTorpedoStart++];
          var torpedo = torpedoProto.clone();
          var dx = Math.cos(torpedoStart.heading) * BerryBots.TORPEDO_SPEED;
          var dy = Math.sin(torpedoStart.heading) * BerryBots.TORPEDO_SPEED;
          torpedo.setX(torpedoStart.srcX);
          torpedo.setY(torpedoStart.srcY);
          torpedo.torpedoData = {dx: dx, dy: dy, id: torpedoStart.id};
          torpedos.push(torpedo);
          layer.add(torpedo);
        }

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
              blastScale = (blastTime - 3) / (BerryBots.TORPEDO_BLAST_TIME - 4);
            }
            torpedoBlastCircle.setScale(blastScale);
          }
        }

        while (nextTorpedoBlast < numTorpedoBlasts
               && torpedoBlasts[nextTorpedoBlast].startTime <= gameTime) {
          var torpedoBlast = torpedoBlasts[nextTorpedoBlast++];
          var torpedoBlastCircle = torpedoBlastProto.clone();
          torpedoBlastCircle.setX(torpedoBlast.x);
          torpedoBlastCircle.setY(torpedoBlast.y);
          torpedoBlastCircle.blastData = torpedoBlast;
          torpedoBlastCircles.push(torpedoBlastCircle);
          layer.add(torpedoBlastCircle);
        }

        for (var x = 0; x < torpedoDebrisCircles.length; x++) {
          var torpedoDebrisCircle = torpedoDebrisCircles[x];
          if (torpedoDebrisCircle.debrisData.endTime < gameTime) {
            torpedoDebrisCircles.splice(x--, 1);
            torpedoDebrisCircle.destroy();
          } else {
            torpedoDebrisCircle.setOffset({x: torpedoDebrisCircle.getOffset().x
                                           + torpedoDebrisCircle.debrisData.speed});
            torpedoDebrisCircle.move(torpedoDebrisCircle.debrisData.dx,
                                    -torpedoDebrisCircle.debrisData.dy);
          }
        }

        while (nextTorpedoDebris < numTorpedoDebris
               && torpedoDebris[nextTorpedoDebris].startTime <= gameTime) {
          var debrisData = torpedoDebris[nextTorpedoDebris++];
          for (var x = 0; x < debrisData.parts; x++) {
            var torpedoDebrisCircle = torpedoDebrisProto.clone();
            torpedoDebrisCircle.setX(debrisData.x);
            torpedoDebrisCircle.setY(debrisData.y);
            torpedoDebrisCircle.setFill(ships[debrisData.shipIndex].shipColor);
            torpedoDebrisCircle.setRotation(Math.random() * BerryBots.TWO_PI);
            torpedoDebrisCircle.setOffset({x: 9});
            torpedoDebrisCircle.debrisData = debrisData;
            torpedoDebrisCircles.push(torpedoDebrisCircle);
            layer.add(torpedoDebrisCircle);
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

        while (nextShipDestroy < numShipDestroys
               && shipDestroys[nextShipDestroy].startTime <= gameTime) {
          var shipDestroy = shipDestroys[nextShipDestroy++];
          var shipDestroyCircle = shipDestroyProto.clone();
          shipDestroyCircle.setX(shipDestroy.x);
          shipDestroyCircle.setY(shipDestroy.y);
          shipDestroyCircle.setStroke(ships[shipDestroy.shipIndex].shipColor);
          shipDestroyCircle.destroyData = shipDestroy;
          shipDestroyCircles.push(shipDestroyCircle);
          layer.add(shipDestroyCircle);
        }

        // Ships.
        while (nextShipAdd < numShipAdds
               && shipAdds[nextShipAdd].time <= gameTime) {
          shipAlives[shipAdds[nextShipAdd++].index] = true;
        }

        while (nextShipRemove < numShipRemoves
               && shipRemoves[nextShipRemove].time <= gameTime) {
          shipAlives[shipRemoves[nextShipRemove++].index] = false;
        }

        while (nextShipShowName < numShipAddShowNames
               && shipAddShowNames[nextShipShowName].time <= gameTime) {
          shipShowNames[shipAddShowNames[nextShipShowName++].index] = true;
        }

        while (nextShipHideName < numShipAddHideNames
               && shipAddHideNames[nextShipHideName].time <= gameTime) {
          shipShowNames[shipAddHideNames[nextShipHideName++].index] = false;
        }

        while (nextShipShowEnergy < numShipAddShowEnergys
               && shipAddShowEnergys[nextShipShowEnergy].time <= gameTime) {
          var shipIndex = shipAddShowEnergys[nextShipShowEnergy++].index;
          shipShowEnergys[shipIndex] = true;
          ships[shipIndex].getChildren()[1].setPosition(0, 24);
        }

        while (nextShipHideEnergy < numShipAddHideEnergys
               && shipAddHideEnergys[nextShipHideEnergy].time <= gameTime) {
          var shipIndex = shipAddHideEnergys[nextShipHideEnergy++].index;
          shipShowEnergys[shipIndex] = false;
          ships[shipIndex].getChildren()[1].setPosition(0, 16);
        }

        for (var x = 0; x < numShips; x++) {
          if (shipAlives[x]) {
            var shipState = shipStates[nextShipState++];
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
                + (BerryBots.SHIP_ROTATION * shipOrbits[x]));
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
          layer.add(stageTextShape);
        }

        while (nextLogEntry < numLogEntries
               && logEntries[nextLogEntry].time <= gameTime) {
          var logEntry = logEntries[nextLogEntry++];
          var console = getConsole(logEntry.teamIndex);
          console.logMessages.push(logEntry.message);
          if (console.div != null) {
            console.outputDiv.innerHTML += '<br>' + logEntry.message;
            scrollToBottom(console.outputDiv);
          }
        }

        gameTime++;
      } while (gameTime < skipTime);
      if (skipTime != -1) {
        overlayPing();
      }
      skipTime = -1;
    } else if (!showedResults) {
      showResults();
      showedResults = true;
    }
  }

  var scale = Math.min(window.innerWidth / stage.getWidth(),
                       window.innerHeight / stage.getHeight());
  stage.setScale(Math.min(1, scale, scale));
}, layer);

anim.start();

function showOverlay() {
  var d = new Date();
  startShowing = d.getTime();

  showConsoleTabs();
  showControls();
  showTimeDisplay();
  showingOverlay = true;
}

function hideOverlay() {
  var d = document.getElementById('dock');
  document.getElementById('container').removeChild(d);
  var f = document.getElementById('controls');
  document.getElementById('container').removeChild(f);
  var g = document.getElementById('timedisplay');
  document.getElementById('container').removeChild(g);
  knob = null;
  showingOverlay = false;
}

function showConsoleTabs() {
  var s = '<style type="text/css">.console-tab { border: 1px solid #fff; '
      + 'padding: 5px; margin: 4px; color: #fff; background-color: #000; } '
      + '.console-tab:hover { color: #0f0; cursor: pointer; '
      + 'border-color: #0f0; } .stage-tab { margin-bottom: 0.7em; }</style>'
      + '<div class="console-tab stage-tab" onclick="showConsole(-1)">' + stageName
      + '</div>';
  for (var x = 0; x < numTeams; x++) {
    var showName = false;
    for (var y = 0; y < numShips; y++) {
      if (ships[y].teamIndex == x && shipShowNames[y]) {
        showName = true;
        break;
      }
    }
    if (showName) {
      s += '<div class="console-tab" onclick="showConsole(' + teams[x].index
          + ')">' + teams[x].name + '</div>';
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
}

function showControls() {
  var s = '<style type="text/css">'
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
        + '</style>'
        + '<div class="controls" onclick="playPause()">'
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

  showPlayPause();
}

function showPlayPause() {
  var center =
      Math.max(0, (stage.getScaleX() * stage.getWidth()) / 2);
  var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 150);

  var d = document.getElementById('controls');
  d.style.position = 'absolute';
  if (paused) {
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
}

function showTimeDisplay() {
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 85);
  var timerWidth = (stageWidth / 2);
  var timerLeft = (stageWidth / 4);
  var s = '<style type="text/css">'
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
      + '</style>'
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
  knob = document.getElementById('timeknob');
  document.getElementById('timetarget').onmousedown = function(e) {
    overlayPing();
    timeMouseDown(e);
  };
  document.getElementById('timetarget').onmouseup = function(e) {
    overlayPing();
    timeMouseUp(e);
  };

  adjustTimeKnob();
}

function adjustTimeKnob() {
  knob.style.top = '6px';
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  if (timeDragging) {
    var left = stageWidth / 4;
    // Visual feedback you can't rewind (for now).
    var minX = (gameTime / endTime) * (stageWidth / 2);
    knob.style.left = (Math.max(minX, (mouseX - left )) + 8) + 'px';
  } else {
    var top = Math.max(0, (stage.getScaleY() * stage.getHeight()) - 75);

    var left = (gameTime / endTime) * (stageWidth / 2) + 8;
    knob.style.left = left + 'px';
  }
}

function timeMouseDown(e) {
  timeDragging = true;
}

function timeMouseUp(e) {
  timeDragging = false;
  var clickX = e.pageX;
  var stageWidth = (stage.getScaleX() * stage.getWidth());
  var left = stageWidth / 4;
  var width = stageWidth / 2;
  var skipFraction = (clickX - left) / width;
  skipTime =
      Math.max(0, Math.min(Math.round(skipFraction * endTime), endTime));
  if (skipTime > gameTime && paused) {
    playPause();
  }
}

function showResults() {
  var s = '<style type="text/css">table, td, th { border-collapse: collapse; '
      + 'background-color: #fff; border: 1px solid #000; color: #000 }'
      + '.num { text-align: right; } .mid { text-align: center; }'
      + '.results-x { font-size: 1.8em; position: absolute; left: 8px; '
      + 'cursor: pointer; top: 0; } .results-x:hover { color: #f00; }'
      + '.rel { position: relative; }'
      + '</style>';
  
  var numResults = results.length;
  var hasScores = false;
  var statKeys = new Array();
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
        + '<div class="results-x" onclick="hideResults()">&times;</div>'
        + 'Results</td></tr><tr><td class="mid">Rank</td>'
        + '<td class="mid">Name</td>';

  if (hasScores) {
    s += '<td class="mid">Score</td>';
  }
  for (var x = 0; x < numKeys; x++) {
    s += '<td class="mid">' + statKeys[x] + '</td>';
  }
  s += '</tr>';

  for (var x = 0; x < numResults; x++) {
    var result = results[x];
    s += '<tr><td class="mid">' + result.rank + '</td><td>'
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
  var left = Math.max(0, ((stage.getScaleX() * stage.getWidth()) - resultsTable.clientWidth) / 2);
  var top = Math.max(0, ((stage.getScaleY() * stage.getHeight()) - resultsTable.clientHeight) / 2);
  d.style.position = 'absolute';
  d.style.left = left + 'px';
  d.style.top = top + 'px';
  resultsDiv = d;
}

function showConsole(teamIndex) {
  for (var x = -1; x < numTeams; x++) {
    var console = getConsole(x);
    if (x != teamIndex && console.showing) {
      hideConsole(x);
    }
  }

  var console = getConsole(teamIndex);
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
        + 'onclick="hideConsole(' + teamIndex + ')">&times;</div>'
        + console.name + '</div><div class="console" id="' + consoleId + '">';

    // TODO: escape or strip HTML
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
  d.style.left =
      Math.max(0, ((stage.getScaleX() * stage.getWidth()) - 600)) + 'px';
  d.style.top = '35px';

  console.showing = true;
  scrollToBottom(console.outputDiv);
}

function hideConsole(teamIndex) {
  var console = getConsole(teamIndex);
  document.getElementById('container').removeChild(console.div);
  console.showing = false;
}

function hideResults() {
  if (resultsDiv != null) {
    document.getElementById('container').removeChild(resultsDiv);
  }
}

function scrollToBottom(d) {
  d.scrollTop = d.scrollHeight;
}

function playPause() {
  paused = !paused;
  showPlayPause();
}


// Functions for parsing replay data into data model.

function drawRectangles(baseOffset, fillColor) {
  var numRectangles = getNumber(baseOffset);
  for (var x = 0; x < numRectangles; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var left = getNumber(offset);
    var bottom = getNumber(offset + 1);
    var width = getNumber(offset + 2);
    var height = getNumber(offset + 3);
    var wallRect = new Kinetic.Rect({
      x: BerryBots.STAGE_MARGIN + left,
      y: BerryBots.STAGE_MARGIN + stageHeight - height - bottom,
      width: width,
      height: height,
      fill: fillColor
    });
    layer.add(wallRect);
  }
  return numRectangles;
}

function getTeamProperties(baseOffset) {
  var numTeams = getNumber(baseOffset);
  var teams = new Array();
  for (var x = 0; x < numTeams; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var team = new Object();
    team.index = getNumber(offset);
    team.name = getString(offset + 1);
    team.logMessages = new Array();
    team.div = null;
    team.showing = false;
    team.consoleId = 'console' + (team.index + 1);
    teams.push(team);
  }
  return teams;
}

function getShipProperties(baseOffset) {
  var numShips = getNumber(baseOffset);
  var ships = new Array();
  for (var x = 0; x < numShips; x++) {
    var offset = baseOffset + 1 + (x * 5);
    var ship = shipGroup.clone();

    ship.teamIndex = getNumber(offset);
    ship.getChildren()[0].setFill(values[offset + 3]);
    var shipName = ship.getChildren()[1];
    shipName.setText(getString(offset + 4));
    shipName.setOffset({x: shipName.getWidth() / 2 });

    ship.getChildren()[3].setStroke(values[offset + 1]);
    var shipDots = ship.getChildren()[4].getChildren();
    shipDots[0].setFill(values[offset + 2]);
    shipDots[1].setFill(values[offset + 2]);
    shipDots[2].setFill(values[offset + 2]);
    ship.shipColor = values[offset + 1];
    ship.laserColor = values[offset + 2];

    ships.push(ship);
    layer.add(ship);
  }
  return ships;
}

function getShipOrbits(numShips) {
  var shipOrbits = new Array();
  var numShips = ships.length;
  for (var x = 0; x < numShips; x++) {
    shipOrbits[x] = (Math.random() < 0.5 ? 1 : -1);
  }
  return shipOrbits;
}

function getShipStateChanges(baseOffset) {
  var numShipStateChanges = getNumber(baseOffset);
  var shipStateChanges = new Array();
  for (var x = 0; x < numShipStateChanges; x++) {
    var offset = baseOffset + 1 + (x * 2);
    shipStateChanges[x] = {index: getNumber(offset), time: getNumber(offset + 1)};
  }
  return shipStateChanges;
}

function getShipStates(baseOffset) {
  var numTicks = getNumber(baseOffset);
  var shipStates = new Array();
  for (var x = 0; x < numTicks; x++) {
    var offset = baseOffset + 1 + (x * 5);
    var shipX = BerryBots.STAGE_MARGIN + getNumber(offset) / 10;
    var shipY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 1) / 10);
    var thrusterAngle = getNumber(offset + 2) / 100;
    var thrusterForce = getNumber(offset + 3) / 100;
    var energy = getNumber(offset + 4) / 10;

    shipStates[x] = {x: shipX, y: shipY, thrusterAngle: thrusterAngle,
                     thrusterForce: thrusterForce, energy: energy};
  }
  return shipStates;
}

function getProjectileStarts(baseOffset) {
  var numProjectileStarts = getNumber(baseOffset);
  var projectileStarts = new Array();
  for (var x = 0; x < numProjectileStarts; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var projectileId = getNumber(offset);
    var shipIndex = getNumber(offset + 1);
    var fireTime = getNumber(offset + 2);
    var srcX = BerryBots.STAGE_MARGIN + getNumber(offset + 3) / 10;
    var srcY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 4) / 10);
    var heading = getNumber(offset + 5) / 100;

    projectileStarts[x] = {id: projectileId, shipIndex: shipIndex,
        fireTime: fireTime, srcX: srcX, srcY: srcY, heading: heading};
  }
  return projectileStarts;
}

function getProjectileEnds(baseOffset) {
  var numProjectileEnds = getNumber(baseOffset);
  var projectileEnds = new Array();
  for (var x = 0; x < numProjectileEnds; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var projectileId = getNumber(offset);
    var endTime = getNumber(offset + 1);
    projectileEnds[x] = {id: projectileId, time: endTime};
  }
  return projectileEnds;
}

function getLaserSparks(baseOffset) {
  var numLaserSparks = getNumber(baseOffset);
  var laserSparks = new Array();
  for (var x = 0; x < numLaserSparks; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var shipIndex = getNumber(offset);
    var time = getNumber(offset + 1);
    var sparkX = BerryBots.STAGE_MARGIN + getNumber(offset + 2) / 10;
    var sparkY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 3) / 10);
    var dx = getNumber(offset + 4) / 100;
    var dy = getNumber(offset + 5) / 100;
    laserSparks[x] = {shipIndex: shipIndex, startTime: time, x: sparkX,
        y: sparkY, dx: dx, dy: dy, endTime: time + BerryBots.LASER_SPARK_TIME};
  }
  return laserSparks;
}

function getTorpedoBlasts(baseOffset) {
  var numTorpedoBlasts = getNumber(baseOffset);
  var torpedoBlasts = new Array();
  for (var x = 0; x < numTorpedoBlasts; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var time = getNumber(offset);
    var blastX = BerryBots.STAGE_MARGIN + getNumber(offset + 1) / 10;
    var blastY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 2) / 10);
    torpedoBlasts[x] = {startTime: time, x: blastX, y: blastY,
                        endTime: time + BerryBots.TORPEDO_BLAST_TIME};
  }
  return torpedoBlasts;
}

function getTorpedoDebris(baseOffset) {
  var numTorpedoDebris = getNumber(baseOffset);
  var torpedoDebris = new Array();
  for (var x = 0; x < numTorpedoDebris; x++) {
    var offset = baseOffset + 1 + (x * 7);
    var shipIndex = getNumber(offset);
    var time = getNumber(offset + 1);
    var debrisX = BerryBots.STAGE_MARGIN + getNumber(offset + 2) / 10;
    var debrisY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 3) / 10);
    var dx = getNumber(offset + 4) / 100;
    var dy = getNumber(offset + 5) / 100;
    var parts = getNumber(offset + 6);
    torpedoDebris[x] = {shipIndex: shipIndex, startTime: time, x: debrisX,
        y: debrisY, dx: dx, dy: dy, parts: parts,
        speed: ((Math.random() + 1) / 2) * 2.5,
        endTime: time + BerryBots.TORPEDO_DEBRIS_TIME};
  }
  return torpedoDebris;
}

function getShipDestroys(baseOffset) {
  var numShipDestroys = getNumber(baseOffset);
  var shipDestroys = new Array();
  for (var x = 0; x < numShipDestroys; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var shipIndex = getNumber(offset);
    var time = getNumber(offset + 1);
    var shipX = BerryBots.STAGE_MARGIN + getNumber(offset + 2) / 10;
    var shipY =
        BerryBots.STAGE_MARGIN + stageHeight - (getNumber(offset + 3) / 10);
    var endTime = time + BerryBots.DESTROY_TIME;
    for (var y = 0; y < BerryBots.DESTROY_CIRCLES; y++) {
      var startTime = time + (y * BerryBots.DESTROY_FRAME_LENGTH);
      shipDestroys.push({shipIndex: shipIndex, startTime: startTime, x: shipX,
                         y: shipY, endTime: endTime});
    }
  }
  return shipDestroys;
}

function getStageTexts(baseOffset) {
  var numStageTexts = getNumber(baseOffset);
  var stageTexts = new Array();
  for (var x = 0; x < numStageTexts; x++) {
    var offset = baseOffset + 1 + (x * 8);
    var time = getNumber(offset);
    var text = getString(offset + 1);
    var size = getNumber(offset + 4);
    var textX = BerryBots.STAGE_MARGIN + getNumber(offset + 2) / 10;
    var textY = BerryBots.STAGE_MARGIN + stageHeight
        - (getNumber(offset + 3) / 10) - size;
    var color = values[offset + 5];
    var opacity = getNumber(offset + 6);
    var duration = getNumber(offset + 7);
    var endTime = time + duration;
    stageTexts[x] = {startTime: time, text: text, x: textX, y: textY,
        size: size, color: color, opacity: opacity,
        endTime: time + duration};
  }
  return stageTexts;
}

function getLogEntries(baseOffset) {
  var numLogEntries = getNumber(baseOffset);
  var logEntries = new Array();
  for (var x = 0; x < numLogEntries; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var teamIndex = getNumber(offset);
    var time = getNumber(offset + 1);
    var message = getString(offset + 2);
    logEntries.push({teamIndex: teamIndex, time: time, message: message});
  }
  return logEntries;
}

function getResults(baseOffset) {
  var offset = baseOffset;
  var numResults = getNumber(offset++);
  var results = new Array();
  for (var x = 0; x < numResults; x++) {
    var teamIndex = getNumber(offset++);
    var rank = getNumber(offset++);
    var score = getNumber(offset++) / 100;
    var numStats = getNumber(offset++);
    var stats = new Array();
    for (var y = 0; y < numStats; y++) {
      var key = getString(offset++);
      var value = getNumber(offset++) / 100;
      stats.push({key: key, value: value});
    }
    results.push(
        {teamIndex: teamIndex, rank: rank, score: score, stats: stats});
  }
  return results;
}

function getConsole(teamIndex) {
  return (teamIndex == -1) ? stageConsole : teams[teamIndex];
}

function getNumber(offset) {
  return parseInt(values[offset], 16);
}

function getString(offset) {
  return values[offset].replace(/@;@/g, ':');
}

function getEndTime(numShips, shipStates, shipAdds, shipRemoves) {
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
}
