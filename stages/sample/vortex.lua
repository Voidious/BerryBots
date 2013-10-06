-- A single player arcade style game. There is a gravity-like force pulling the
-- player ship towards the center of the stage and meteors (stage ships) getting
-- sucked into the vortex to obstruct its path.
--
-- If a ship gets pulled too close to the center, it is destroyed. Since the
-- gravity force is stronger than the ship's maximum thruster force, you must
-- slingshot around the center to survive.
--
-- Vortexer is the only sample ship designed for this stage.

require "samplestage"

START_DISTANCE = 150
MAX_METEORS = 20
NUM_ROUNDS = 25

function userShipStart()
  local startBearing = math.random() * 2 * math.pi
  local startX = 500 + (math.cos(startBearing) * START_DISTANCE)
  local startY = 500 + (math.sin(startBearing) * START_DISTANCE)
  return {x = startX, y = startY}
end

function configure(stageBuilder)
  stageBuilder:setSize(1000, 1000)
  stageBuilder:setBattleMode(true)
  stageBuilder:addWall(100, 100, 80, 80)
  stageBuilder:addWall(820, 100, 80, 80)
  stageBuilder:addWall(820, 820, 80, 80)
  stageBuilder:addWall(100, 820, 80, 80)
  stageBuilder:addZone(0, 0, 40, 40)
  stageBuilder:addZone(960, 0, 40, 40)
  stageBuilder:addZone(0, 960, 40, 40)
  stageBuilder:addZone(960, 960, 40, 40)

  local start = userShipStart()
  stageBuilder:addStart(start.x, start.y)

  for i = 1, MAX_METEORS do
    stageBuilder:addShip("vortexmeteor.lua")
  end
end

userShip = nil
stageShips = { }
shipSettings = { }
center = nil
wallLines = nil
round = 1
roundsWon = 0
drawNiceTimer = 0

PULL_FORCE = 1.5
DEATH_DISTANCE = 50
METEOR_FREQUENCY = 10
METEOR_ENERGY = 20
METEOR_SPEED = 10
METEOR_WALL_DISTANCE = 25

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg
  for i, ship in ipairs(ships) do
    if (ship:isStageShip()) then
      table.insert(stageShips, ship)
      admin:setShipShowName(ship, false)
    elseif (userShip == nil) then
      userShip = ship
    end
  end
  center = {x = (world:width() / 2), y = (world:height() / 2)}
  wallLines = initWalls(world:walls())
end

function initWalls(walls)
  local wallLines = { }
  for i, wall in pairs(walls) do
    local x1, y1 = wall.left, wall.bottom
    local x2, y2 = wall.left + wall.width, wall.bottom
    local x3, y3 = wall.left + wall.width, wall.bottom + wall.height
    local x4, y4 = wall.left, wall.bottom + wall.height
    table.insert(wallLines, makeLine(x1, y1, x2, y2))
    table.insert(wallLines, makeLine(x2, y2, x3, y3))
    table.insert(wallLines, makeLine(x3, y3, x4, y4))
    table.insert(wallLines, makeLine(x4, y4, x1, y1))
  end
  return wallLines
end

function run(stageSensors)
  samplestage.checkSinglePlayer(ships, admin)

  if (world:time() % METEOR_FREQUENCY == 0) then
    local meteorShip = nextMeteor()
    if (meteorShip ~= nil) then
      local meteorPosition = meteorStart()
      admin:moveShip(meteorShip, meteorPosition.x, meteorPosition.y)
      admin:reviveShip(meteorShip)
      admin:setShipEnergy(meteorShip, METEOR_ENERGY)
      local bearingToCenter =
          math.atan2(center.y - meteorShip:y(), center.x - meteorShip:x())
      admin:setShipHeading(meteorShip, bearingToCenter)
      admin:setShipSpeed(meteorShip, METEOR_SPEED)
    end
  end

  for i, ship in pairs(ships) do
    if (ship:alive()) then
      if (ship:isStageShip()) then
        if (ship:energy() <= 1) then
          admin:destroyShip(ship)
        else
          admin:setShipEnergy(ship, ship:energy() - .1)
        end
      end
      local distanceToCenter =
          math.sqrt(square(center.x - ship:x()) + square(center.y - ship:y()))
      if (distanceToCenter < DEATH_DISTANCE) then
        admin:destroyShip(ship)
      else
        local bearingToCenter =
            math.atan2(center.y - ship:y(), center.x - ship:x())
        local speedX = ship:speed() * math.cos(ship:heading())
        local speedY = ship:speed() * math.sin(ship:heading())
        speedX = speedX + (PULL_FORCE * math.cos(bearingToCenter))
        speedY = speedY + (PULL_FORCE * math.sin(bearingToCenter))
        admin:setShipSpeed(ship, math.sqrt(square(speedX) + square(speedY)))
        admin:setShipHeading(ship, math.atan2(speedY, speedX))
      end
    end
  end

  local endRound = false
  if (world:touchedAnyZone(userShip)) then
    roundsWon = roundsWon + 1
    drawNiceTimer = drawNiceTimer + 90
    endRound = true
  end
  if (not userShip:alive()) then
    endRound = true
  end
  if (endRound) then
    if (round == NUM_ROUNDS) then
      print(userShip:name() .. " score: " .. roundsWon .. " / " .. NUM_ROUNDS)
      admin:setScore(userShip, roundsWon)
      admin:gameOver()
    else
      admin:roundOver()
      local start = userShipStart()
      admin:moveShip(userShip, start.x, start.y)
    end
    round = round + 1
  end

  admin:drawText("Score: " .. roundsWon .. " / " .. (round - 1), 415, 950)
  if (drawNiceTimer > 0) then
    admin:drawText("Nice!", 475, 50)
    drawNiceTimer = drawNiceTimer - 1
  end
end

function nextMeteor()
  for i, ship in pairs(stageShips) do
    if (not ship:alive()) then
      return ship
    end
  end
  return nil
end

function meteorStart()
  local meteorX
  local meteorY
  repeat
    meteorX = math.random() * METEOR_WALL_DISTANCE
    if (math.random() < 0.5) then
      meteorX = 1000 - meteorX
    end
    meteorY = math.random() * 1000
    if (math.random() < 0.5) then
      meteorX, meteorY = meteorY, meteorX
    end
  until isVisible(meteorX, meteorY, center.x, center.y)
  return {x = meteorX, y = meteorY}
end

function makeLine(x1, y1, x2, y2)
  local wallLine = { }
  if (x1 == x2) then
    wallLine.m = math.huge
    wallLine.b = math.huge
    wallLine.xMin, wallLine.xMax = x1, x1
  else
    wallLine.m = (y2 - y1) / (x2 - x1)
    wallLine.b = y1 - (wallLine.m * x1)
    wallLine.xMin = math.min(x1, x2)
    wallLine.xMax = math.max(x1, x2)
  end
  wallLine.yMin = math.min(y1, y2)
  wallLine.yMax = math.max(y1, y2)
  wallLine.x1, wallLine.y1, wallLine.x2, wallLine.y2 = x1, y1, x2, y2

  return wallLine
end

function isVisible(x1, y1, x2, y2)
  local visionLine = makeLine(x1, y1, x2, y2)
  for i, wallLine in pairs(wallLines) do
    if (intersects(wallLine, visionLine)) then
      return false
    end
  end
  return true
end

function intersects(line1, line2)
  local m1 = line1.m
  local m2 = line2.m
  if (m1 == m2) then
    return false
  elseif (m1 == math.huge or m2 == math.huge) then
    if (m1 == math.huge and m2 == 0) then
      return (line1.xMin >= line2.xMin and line1.xMax <= line2.xMax
              and line1.yMin <= line2.yMin and line1.yMax >= line2.yMax)
    else
      return intersects(inverse(line1), inverse(line2))
    end
  end
  
  local x = (line2.b - line1.b) / (line1.m - line2.m)
  return (x >= line1.xMin and x <= line1.xMax and x >= line2.xMin
      and x <= line2.xMax)
end

function inverse(line)
  return makeLine(line.y1, line.x1, line.y2, line.x2)
end

function square(x)
  return x * x
end
