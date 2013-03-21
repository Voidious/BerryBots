-- A ship that moves to a wall and then follows the wall without hitting it. If
-- battle mode is enabled, also shoots at the nearest visible ship.
--
-- Can naively solve some mazes (such as the samples), and it's also a
-- reasonable free-for-all battle strategy. Can also be initialized as a team of
-- any size, controlling each ship with the same logic and totally ignoring
-- teammates.

require "battlebot"

MAX_SPEED = 8

ships = { }
shipStates = { }
wallLines = nil
constants = nil

function init(shipsArg, worldArg)
  if (type(shipsArg) == "userdata") then
    table.insert(ships, shipsArg)
  else
    ships = shipsArg
    ships[1]:setTeamName("WallHuggerTeam")
  end
  world = worldArg

  for i, ship in pairs(ships) do
    ship:setName("WallHugger")
    ship:setShipColor(0, 255, 0)
    ship:setLaserColor(255, 255, 255)
    ship:setThrusterColor(0, 255, 0)
    local shipState = {heading=randomDirection(), started = false,
                       stopping = false, wasAgainstWall = true}
    shipStates[ship] = shipState
  end

  wallLines = battlebot.initWalls(world:walls())
  constants = world:constants()
end

function run(enemyShips, sensors)
  for i, ship in pairs(ships) do
    runShip(ship, enemyShips, sensors)
  end
end

function runShip(ship, enemyShips, sensors)
  moveAlongWalls(ship)
  fireAtNearestEnemy(ship, enemyShips)
end

function moveAlongWalls(ship)
  local state = shipStates[ship]
  if (state.stopping and round(ship:speed(), 2) == 0) then
    state.stopping = false
    state.started = false
    state.heading = randomDirection()
  elseif (state.stopping or ship:hitWall() or ship:hitShip()) then
    state.stopping = true
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  else
    if (state.started and round(ship:speed(), 2) == 0) then
      state.started = false
      local turn = 1
      local aheadWallFeeler =
          feelerLine(ship, state.heading, constants.SHIP_RADIUS + 1, 0)
      if (wouldHitWall(wallLines, aheadWallFeeler)) then
        turn = turn * -1
      end

      state.heading =
          (state.heading + (turn * (math.pi / 2))) % (2 * math.pi)
      state.wasAgainstWall = false
    end
  
    local accelDistance = minDistanceForSpeed(ship:speed() + 1)
    local coastDistance = minDistanceForSpeed(ship:speed())
    local decelDistance = minDistanceForSpeed(math.max(0, ship:speed() - 1))
    local accelLine = moveLine(ship, state.heading, accelDistance)

    local hereWallFeeler = feelerLine(ship, state.heading, 0, 1)
    local decelWallFeeler = feelerLine(ship, state.heading, decelDistance, 1)
    local coastWallFeeler = feelerLine(ship, state.heading, coastDistance, 1)

    local againstWall = wouldHitWall(wallLines, hereWallFeeler)
    state.wasAgainstWall = state.wasAgainstWall or againstWall

    if (state.started and state.wasAgainstWall
        and not wouldHitWall(wallLines, decelWallFeeler)) then
      -- Decelerate to follow concave corner.
      ship:fireThruster(ship:heading() + math.pi, ship:speed())
    elseif (state.started and state.wasAgainstWall
        and not wouldHitWall(wallLines, coastWallFeeler)) then
      -- Coast to follow concave corner. (Do nothing.)
    elseif (not wouldHitWall(wallLines, accelLine)) then
      -- Safe to accelerate in current direction.
      ship:fireThruster(state.heading, MAX_SPEED - ship:speed())
    elseif (round(ship:speed(), 2) == 0) then
      -- No valid options in current direction, select new random direction.
      state.heading = randomDirection()
      ship:fireThruster(state.heading, 1)
      local newWallFeeler = feelerLine(ship, state.heading, 0, 1)
      local againstWall = wouldHitWall(wallLines, hereWallFeeler)
      state.wasAgainstWall = state.wasAgainstWall or againstWall
    else
      local coastLine = moveLine(ship, state.heading, coastDistance)
      if (wouldHitWall(wallLines, coastLine)) then
        ship:fireThruster(ship:heading() + math.pi, ship:speed())
      end
    end
    state.started = true
  end
end

function minDistanceForSpeed(initialSpeed)
  local minTicks = math.ceil(initialSpeed)
  return minTicks * ((initialSpeed + 1) / 2)
end

function moveLine(ship, heading, distance)
  local target = project({x=ship:x(), y=ship:y()}, heading, distance)
  return battlebot.newLine(ship:x(), ship:y(), target.x, target.y)
end

function feelerLine(ship, heading, distance, orientation)
  local source = project({x=ship:x(), y=ship:y()}, heading, distance)
  local target = project(source, heading + (orientation * (math.pi / 2)),
      constants.SHIP_RADIUS + 1)
  return battlebot.newLine(source.x, source.y, target.x, target.y)
end

function project(source, angle, distance)
  local target = { }
  target.x = source.x + (math.cos(angle) * distance)
  target.y = source.y + (math.sin(angle) * distance)
  return target
end

function wouldHitWall(wallLines, moveLine)
  for i, wallLine in pairs(wallLines) do
    if (battlebot.intersects(wallLine, moveLine, constants.SHIP_RADIUS + 1)) then
      return true
    end
  end
  return false
end

function round(d, x)
  local powerTen = 1
  for i = 1, x do
    powerTen = powerTen * 10
  end
  return math.floor((d * powerTen) + .5) / powerTen
end

function randomDirection()
  local i = math.floor(math.random() * 4)
  return i * math.pi / 2
end

function fireAtNearestEnemy(ship, enemyShips)
  local targetShip = nil
  local nearestDistanceSq = math.huge
  for i, enemyShip in pairs(enemyShips) do
    local distSq = distanceSqToEnemy(ship, enemyShip)
    if (distSq < nearestDistanceSq) then
      nearestDistanceSq = distSq
      targetShip = enemyShip
    end
  end
  if (targetShip ~= nil) then
    local targetDistance = math.sqrt(nearestDistanceSq)
    local firingAngle = math.atan2(targetShip.y - ship:y(),
                                   targetShip.x - ship:x())
    ship:fireLaser(firingAngle)
    ship:fireTorpedo(firingAngle, targetDistance)
  end
end

function distanceSqToEnemy(ship, enemyShip)
  return square(ship:x() - enemyShip.x) + square(ship:y() - enemyShip.y)
end

function square(x)
  return x * x
end
