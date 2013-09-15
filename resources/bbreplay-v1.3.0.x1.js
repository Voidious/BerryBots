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

var STAGE_MARGIN = 25;
var LASER_SPEED = 25;
var TORPEDO_SPEED = 12;
var SHIP_ROTATION = Math.PI * 2 / 240;
var THRUSTER_ZERO = 8.3 / 19.3;
var TWO_PI = 2 * Math.PI;
var ZONE_COLOR = "#644444";
var TORPEDO_COLOR = "#ff5926";
var LASER_SPARK_LENGTH = 8;
var LASER_SPARK_THICKNESS = 1.5;
var LASER_SPARK_TIME = 8;
var TORPEDO_RADIUS = 4
var TORPEDO_BLAST_RADIUS = 100;
var TORPEDO_BLAST_TIME = 16;
var TORPEDO_DEBRIS_RADIUS = 1.5;
var TORPEDO_DEBRIS_TIME = 40;
var DESTROY_BASE_RADIUS = 6;
var DESTROY_CIRCLES = 3;
var DESTROY_FRAMES = 16;
var DESTROY_FRAME_LENGTH = 2;
var DESTROY_TIME = (DESTROY_FRAMES * DESTROY_FRAME_LENGTH);


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
  fill: "#00ff00",
  strokeWidth: 0
});

var shipDot2 = new Kinetic.Circle({
  x: -2.845,
  y: 1.6425,
  radius: 1.46,
  fill: "#00ff00",
  strokeWidth: 0
});

var shipDot3 = new Kinetic.Circle({
  x: 2.845,
  y: 1.6425,
  radius: 1.46,
  fill: "#00ff00",
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
  radius: DESTROY_BASE_RADIUS,
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
  width: LASER_SPARK_LENGTH,
  height: LASER_SPARK_THICKNESS,
  fill: '#00ff00',
  strokeWidth: 0,
  offset: [0, LASER_SPARK_THICKNESS / 2]
});

var torpedoCircle = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: TORPEDO_RADIUS,
  fill: TORPEDO_COLOR,
  strokeWidth: 0
});

var torpedoRay = new Kinetic.Rect({
  x: 0,
  y: 0,
  width: TORPEDO_RADIUS * 4,
  height: 1,
  fill: TORPEDO_COLOR,
  offset: [TORPEDO_RADIUS * 2, 0.5]
});

var torpedoBlastProto = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: TORPEDO_BLAST_RADIUS,
  stroke: TORPEDO_COLOR,
  strokeWidth: 5
});

var torpedoDebrisProto = new Kinetic.Circle({
  x: 0,
  y: 0,
  radius: TORPEDO_DEBRIS_RADIUS,
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
  y: 24,
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

var values = replayData.replace(/\\:/g, "@;@").split(":");
var stageName = getValue(1);
var stageWidth = getValue(2);
var stageHeight = getValue(3);

var bgRect = new Kinetic.Rect({
  x: 0,
  y: 0,
  width: stageWidth + (STAGE_MARGIN * 2),
  height: stageHeight + (STAGE_MARGIN * 2),
  fill: 'black',
});
layer.add(bgRect);

var stage = new Kinetic.Stage({
  container: 'container',
  width: stageWidth + (STAGE_MARGIN * 2),
  height: stageHeight + (STAGE_MARGIN * 2)
});

var numWalls = drawRectangles(4, 'white');
var zonesOffset = 5 + (numWalls * 4);
var numZones = drawRectangles(zonesOffset, ZONE_COLOR);

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

var shipStatesOffset =
    shipAddHideNamesOffset + 1 + (numShipAddHideNames * 2);
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

var stageTextsOffset =
    shipDestroysOffset + 1 + ((numShipDestroys / DESTROY_CIRCLES) * 4);
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

var gameTime = 0;
var nextShipState = 0;
var nextShipAdd = 0;
var nextShipRemove = 0;
var nextShipShowName = 0;
var nextShipHideName = 0;
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

var anim = new Kinetic.Animation(function(frame) {
  var time = frame.time,
      timeDiff = frame.timeDiff,
      frameRate = frame.frameRate;

  if (nextShipState < numShipStates) {
    // Lasers.
    while (nextLaserStart < numLaserStarts
           && laserStarts[nextLaserStart].fireTime <= gameTime) {
      var laserStart = laserStarts[nextLaserStart++];
      var laser = laserProto.clone();
      var dx = Math.cos(laserStart.heading) * LASER_SPEED;
      var dy = Math.sin(laserStart.heading) * LASER_SPEED;
      laser.setX(laserStart.srcX);
      laser.setY(laserStart.srcY);
      laser.setFill(ships[laserStart.shipIndex].laserColor);
      laser.setRotation(TWO_PI - laserStart.heading);
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
                                  y: LASER_SPARK_THICKNESS / 2});
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
        laserSparkRect.setRotation(Math.random() * TWO_PI);
        laserSparkRect.setOffset({x: 9, y: LASER_SPARK_THICKNESS / 2});
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
      var dx = Math.cos(torpedoStart.heading) * TORPEDO_SPEED;
      var dy = Math.sin(torpedoStart.heading) * TORPEDO_SPEED;
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
          blastScale = (blastTime - 3) / (TORPEDO_BLAST_TIME - 4);
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
        torpedoDebrisCircle.setRotation(Math.random() * TWO_PI);
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
        shipDestroyCircle.setRadius(radiusFactor * DESTROY_BASE_RADIUS);
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

    for (var x = 0; x < numShips; x++) {
      if (shipAlives[x]) {
        var shipState = shipStates[nextShipState++];
        ships[x].show();
        ships[x].setX(shipState.x);
        ships[x].setY(shipState.y);
        var thruster = ships[x].getChildren()[0];
        thruster.setRotation(Math.PI + (TWO_PI - shipState.thrusterAngle));
        var forceFactor = shipState.thrusterForce;
        if (forceFactor < 0.0001) {
          thruster.setScale(0);
        } else {
          thruster.setScale(
              THRUSTER_ZERO + (forceFactor * (1 - THRUSTER_ZERO)));
        }

        ships[x].getChildren()[2].setScale(shipState.energy / 100, 1);
        var shipDotGroup = ships[x].getChildren()[4];
        shipDotGroup.setRotation(
            shipDotGroup.getRotation() + (SHIP_ROTATION * shipOrbits[x]));
        ships[x].getChildren()[1].setVisible(shipShowNames[x]);
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

    gameTime++;
  }

  var scale = Math.min(window.innerWidth / stage.getWidth(),
                       window.innerHeight / stage.getHeight());
  stage.setScale(Math.min(1, scale, scale));
}, layer);

anim.start();


function drawRectangles(baseOffset, fillColor) {
  var numRectangles = getValue(baseOffset);
  for (var x = 0; x < numRectangles; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var left = getValue(offset);
    var bottom = getValue(offset + 1);
    var width = getValue(offset + 2);
    var height = getValue(offset + 3);
    var wallRect = new Kinetic.Rect({
      x: STAGE_MARGIN + left,
      y: STAGE_MARGIN + stageHeight - height - bottom,
      width: width,
      height: height,
      fill: fillColor
    });
    layer.add(wallRect);
  }
  return numRectangles;
}

function getTeamProperties(baseOffset) {
  var numTeams = getValue(baseOffset);
  var teams = new Array();
  for (var x = 0; x < numTeams; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var team = {index: getValue(offset)};
    team.name = values[offset + 1].replace(/@;@/g, ":");
    teams.push(team);
  }
  return teams;
}

function getShipProperties(baseOffset) {
  var numShips = getValue(baseOffset);
  var ships = new Array();
  for (var x = 0; x < numShips; x++) {
    var offset = baseOffset + 1 + (x * 5);
    ships[x] = shipGroup.clone();

    ships[x].teamIndex = getValue(offset);
    ships[x].getChildren()[0].setFill(values[offset + 3]);
    var shipName = ships[x].getChildren()[1];
    shipName.setText(values[offset + 4].replace(/@;@/g, ":"));
    shipName.setOffset({x: shipName.getWidth() / 2 });

    ships[x].getChildren()[3].setStroke(values[offset + 1]);
    var shipDots = ships[x].getChildren()[4].getChildren();
    shipDots[0].setFill(values[offset + 2]);
    shipDots[1].setFill(values[offset + 2]);
    shipDots[2].setFill(values[offset + 2]);
    ships[x].shipColor = values[offset + 1];
    ships[x].laserColor = values[offset + 2];

    layer.add(ships[x]);
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
  var numShipStateChanges = getValue(baseOffset);
  var shipStateChanges = new Array();
  for (var x = 0; x < numShipStateChanges; x++) {
    var offset = baseOffset + 1 + (x * 2);
    shipStateChanges[x] = {index: getValue(offset), time: getValue(offset + 1)};
  }
  return shipStateChanges;
}

function getShipStates(baseOffset) {
  var numTicks = getValue(baseOffset);
  var shipStates = new Array();
  for (var x = 0; x < numTicks; x++) {
    var offset = baseOffset + 1 + (x * 5);
    var shipX = STAGE_MARGIN + getValue(offset) / 10;
    var shipY =
        STAGE_MARGIN + stageHeight - (getValue(offset + 1) / 10);
    var thrusterAngle = getValue(offset + 2) / 100;
    var thrusterForce = getValue(offset + 3) / 100;
    var energy = getValue(offset + 4) / 10;

    shipStates[x] = {x: shipX, y: shipY, thrusterAngle: thrusterAngle,
                     thrusterForce: thrusterForce, energy: energy};
  }
  return shipStates;
}

function getProjectileStarts(baseOffset) {
  var numProjectileStarts = getValue(baseOffset);
  var projectileStarts = new Array();
  for (var x = 0; x < numProjectileStarts; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var projectileId = getValue(offset);
    var shipIndex = getValue(offset + 1);
    var fireTime = getValue(offset + 2);
    var srcX = STAGE_MARGIN + getValue(offset + 3) / 10;
    var srcY = STAGE_MARGIN + stageHeight - (getValue(offset + 4) / 10);
    var heading = getValue(offset + 5) / 100;

    projectileStarts[x] = {id: projectileId, shipIndex: shipIndex,
        fireTime: fireTime, srcX: srcX, srcY: srcY, heading: heading};
  }
  return projectileStarts;
}

function getProjectileEnds(baseOffset) {
  var numProjectileEnds = getValue(baseOffset);
  var projectileEnds = new Array();
  for (var x = 0; x < numProjectileEnds; x++) {
    var offset = baseOffset + 1 + (x * 2);
    var projectileId = getValue(offset);
    var endTime = getValue(offset + 1);
    projectileEnds[x] = {id: projectileId, time: endTime};
  }
  return projectileEnds;
}

function getLaserSparks(baseOffset) {
  var numLaserSparks = getValue(baseOffset);
  var laserSparks = new Array();
  for (var x = 0; x < numLaserSparks; x++) {
    var offset = baseOffset + 1 + (x * 6);
    var shipIndex = getValue(offset);
    var time = getValue(offset + 1);
    var sparkX = STAGE_MARGIN + getValue(offset + 2) / 10;
    var sparkY = STAGE_MARGIN + stageHeight - (getValue(offset + 3) / 10);
    var dx = getValue(offset + 4) / 100;
    var dy = getValue(offset + 5) / 100;
    laserSparks[x] = {shipIndex: shipIndex, startTime: time, x: sparkX,
        y: sparkY, dx: dx, dy: dy, endTime: time + LASER_SPARK_TIME};
  }
  return laserSparks;
}

function getTorpedoBlasts(baseOffset) {
  var numTorpedoBlasts = getValue(baseOffset);
  var torpedoBlasts = new Array();
  for (var x = 0; x < numTorpedoBlasts; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var time = getValue(offset);
    var blastX = STAGE_MARGIN + getValue(offset + 1) / 10;
    var blastY = STAGE_MARGIN + stageHeight - (getValue(offset + 2) / 10);
    torpedoBlasts[x] = {startTime: time, x: blastX, y: blastY,
                        endTime: time + TORPEDO_BLAST_TIME};
  }
  return torpedoBlasts;
}

function getTorpedoDebris(baseOffset) {
  var numTorpedoDebris = getValue(baseOffset);
  var torpedoDebris = new Array();
  for (var x = 0; x < numTorpedoDebris; x++) {
    var offset = baseOffset + 1 + (x * 7);
    var shipIndex = getValue(offset);
    var time = getValue(offset + 1);
    var debrisX = STAGE_MARGIN + getValue(offset + 2) / 10;
    var debrisY = STAGE_MARGIN + stageHeight - (getValue(offset + 3) / 10);
    var dx = getValue(offset + 4) / 100;
    var dy = getValue(offset + 5) / 100;
    var parts = getValue(offset + 6);
    torpedoDebris[x] = {shipIndex: shipIndex, startTime: time, x: debrisX,
        y: debrisY, dx: dx, dy: dy, parts: parts,
        speed: ((Math.random() + 1) / 2) * 2.5,
        endTime: time + TORPEDO_DEBRIS_TIME};
  }
  return torpedoDebris;
}

function getShipDestroys(baseOffset) {
  var numShipDestroys = getValue(baseOffset);
  var shipDestroys = new Array();
  for (var x = 0; x < numShipDestroys; x++) {
    var offset = baseOffset + 1 + (x * 4);
    var shipIndex = getValue(offset);
    var time = getValue(offset + 1);
    var shipX = STAGE_MARGIN + getValue(offset + 2) / 10;
    var shipY = STAGE_MARGIN + stageHeight - (getValue(offset + 3) / 10);
    var endTime = time + DESTROY_TIME;
    for (var y = 0; y < DESTROY_CIRCLES; y++) {
      var startTime = time + (y * DESTROY_FRAME_LENGTH);
      shipDestroys.push({shipIndex: shipIndex, startTime: startTime, x: shipX,
                         y: shipY, endTime: endTime});
    }
  }
  return shipDestroys;
}

function getStageTexts(baseOffset) {
  var numStageTexts = getValue(baseOffset);
  var stageTexts = new Array();
  for (var x = 0; x < numStageTexts; x++) {
    var offset = baseOffset + 1 + (x * 8);
    var time = getValue(offset);
    var text = values[offset + 1].replace(/@;@/g, ":");
    var size = getValue(offset + 4);
    var textX = STAGE_MARGIN + getValue(offset + 2) / 10;
    var textY = STAGE_MARGIN + stageHeight - (getValue(offset + 3) / 10) - size;
    var color = values[offset + 5];
    var opacity = getValue(offset + 6);
    var duration = getValue(offset + 7);
    var endTime = time + duration;
    stageTexts[x] = {startTime: time, text: text, x: textX, y: textY,
        size: size, color: color, opacity: opacity,
        endTime: time + duration};
  }
  return stageTexts;
}

function getLogEntries(baseOffset) {
  var numLogEntries = getValue(baseOffset);
  var logEntries = new Array();
  for (var x = 0; x < numLogEntries; x++) {
    var offset = baseOffset + 1 + (x * 3);
    var teamIndex = getValue(offset);
    var time = getValue(offset + 1);
    var message = values[offset + 2].replace(/@;@/g, ":");
    logEntries.push({teamIndex: teamIndex, time: time, message: message});
  }
  return logEntries;
}

function getResults(baseOffset) {
  var offset = baseOffset;
  var numResults = getValue(offset++);
  var results = new Array();
  for (var x = 0; x < numResults; x++) {
    var teamIndex = getValue(offset++);
    var rank = getValue(offset++);
    var score = getValue(offset++) / 100;
    var numStats = getValue(offset++);
    var stats = new Array();
    for (var y = 0; y < numStats; y++) {
      var key = values[offset++].replace(/@;@/g, ":");
      var value = getValue(offset++) / 100;
      stats.push({key: key, value: value});
    }
    results.push(
        {teamIndex: teamIndex, rank: rank, score: score, stats: stats});
  }
  return results;
}

function getValue(offset) {
  return parseInt(values[offset], 16);
}
