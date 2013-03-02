-- A target ship for the sample.lasergallery targeting challenge stage.
-- Moves along walls without getting too close.

MAX_SPEED = 12

EAST = 0
NORTH = 1
WEST = 2
SOUTH = 3

ship = nil
world = nil
constants = nil
orientation = 1
targetWall = NORTH

function init(shipsArg, worldArg)
  ship = shipsArg
  world = worldArg
  constants = world:constants()
  ship:setName("Wallsy")
  ship:setShipColor(25, 255, 25)
  ship:setLaserColor(25, 255, 25)
  ship:setThrusterColor(25, 255, 25)

  if (math.random() < 0.5) then
    orientation = -1
  end
  if (math.random() < 0.5) then
    targetWall = SOUTH
  end
end

function run(enemyShips, sensors)
  if (atWall(targetWall)) then
    if (targetWall == WEST) then
      print("Reverse from " .. orientation)
      orientation = orientation * -1
    end
    targetWall = nextWall(targetWall)
  else
    moveToWall(targetWall)
  end
end

function nextWall(wall)
  if (wall == WEST) then
    return EAST
  else
    if (wall == EAST) then
      if (orientation == 1) then
        return SOUTH
      else
        return NORTH
      end
    elseif ((wall == NORTH and orientation == -1)
            or (wall == SOUTH and orientation == 1)) then
      return WEST
    elseif ((wall == SOUTH and orientation == -1)
            or (wall == NORTH and orientation == 1)) then
      return EAST
    else
      print("Don't understand nextWall: " .. wall .. " / " .. orientation)
      return EAST
    end
  end
end

function atWall(wall)
  if (ship:speed() < 0.0001) then
    if ((wall == NORTH and ship:y() > world:height() - 20)
        or (wall == EAST and ship:x() > world:width() - 20)
        or (wall == SOUTH and ship:y() < 20)
        or (wall == WEST and ship:x() < 670)) then
      return true
    end
  end
  return false
end

function moveToWall(wall)
  if (atWall(wall)) then
    wall = nextWall
  else
    local accelDistance = minDistanceForSpeed(math.max(0, ship:speed() + 1))
    local coastDistance = minDistanceForSpeed(math.max(0, ship:speed()))
    local wallDistance = distanceToWall(wall)
    if (accelDistance < wallDistance) then
      -- accelerate
      ship:fireThruster(wallBearing(wall), MAX_SPEED - ship:speed())
    elseif (coastDistance < wallDistance) then
      -- coast
    else
      -- stop
      ship:fireThruster(ship:heading() + math.pi, ship:speed())
    end
  end
end

function distanceToWall(wall)
  if (wall == NORTH) then
    return world:height() - constants.SHIP_RADIUS - ship:y()
  elseif (wall == EAST) then
    return world:width() - constants.SHIP_RADIUS - ship:x()
  elseif (wall == SOUTH) then
    return ship:y() - constants.SHIP_RADIUS
  elseif (wall == WEST) then
    return ship:x() - constants.SHIP_RADIUS - 650
  else
    print("Don't understand wall: " .. wall)
    return 0
  end
end

function wallBearing(wall)
  return (wall * math.pi / 2)
end

function minDistanceForSpeed(initialSpeed)
  local minTicks = math.ceil(initialSpeed)
  return minTicks * ((initialSpeed + 1) / 2)
end
