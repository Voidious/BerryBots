-- A basic battle ship that tries to move to a safe spot on the battle field
-- and aims with random linear targeting.

require "battlebot"

ship = nil
worldWidth = nil
worldHeight = nil
world = nil
wallLines = nil
constants = nil
shipData = { }
newRound = false

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  worldWidth = world:width()
  worldHeight = world:height()
  constants = world:constants()
  wallLines = battlebot.initWalls(world:walls())

  ship:setName("BasicBattler")
  ship:setShipColor(75, 175, 255)
  ship:setLaserColor(200, 200, 0)
  ship:setThrusterColor(200, 200, 0)
end

function run(enemyShips, sensors)
  updateEnemyShipData(enemyShips, sensors)
  moveInSafestDirection()
  fireAtJuiciestTarget()
end

function roundOver()
  newRound = true
end

function updateEnemyShipData(enemyShips, sensors)
  local time = world:time()

  for i, enemyShip in pairs(shipData) do
    enemyShip.visible = false
    if (newRound) then
      enemyShip.lastSeenTime = -100
    end
  end
  newRound = false

  for i, enemyShip in pairs(enemyShips) do
    local name = enemyShip.name
    shipData[name] = enemyShip
    shipData[name].alive = true
    shipData[name].visible = true
    shipData[name].lastSeenTime = time
  end

  for i, shipDestroyed in pairs(sensors:shipDestroyedEvents()) do
    local name = shipDestroyed.shipName
    if (shipData[name] ~= nil) then
      shipData[name].alive = false
    end
  end

  for i, enemyShip in pairs(shipData) do
    if (enemyShip.alive and not enemyShip.visible
        and ((time - enemyShip.lastSeenTime > 20)
            or battlebot.isVisible(
                wallLines, ship, enemyShip.x, enemyShip.y))) then
      -- Don't know where they are and our last data for them is wrong or very
      -- stale, so just guess. At least have a chance of hitting with a torpedo
      -- and keeps us moving around randomly instead of hitting a local minimum.
      for x = 1, 100 do
        local randomX = math.random() * worldWidth
        local randomY = math.random() * worldHeight
        if (not battlebot.isVisible(wallLines, ship, randomX, randomY)) then
          enemyShip.x = randomX
          enemyShip.y = randomY
          enemyShip.speed = 0
          break
        end
      end
    end
  end
end

NUM_SLICES = 12
TICKS_OUT = 5

function moveInSafestDirection()
  local xStart = ship:x()
  local yStart = ship:y()
  local xStartSpeed = math.cos(ship:heading()) * ship:speed()
  local yStartSpeed = math.sin(ship:heading()) * ship:speed()
  local bestRisk = math.huge
  local xBest = xStart
  local yBest = yStart
  local bestAngle = -999
  for x = 1, NUM_SLICES do
    local angle = (x / NUM_SLICES) * 2 * math.pi
    local xEnd = xStart
    local yEnd = yStart
    local xSpeed = xStartSpeed
    local ySpeed = yStartSpeed
    local cosAngle = math.cos(angle)
    local sinAngle = math.sin(angle)
    for y = 1, TICKS_OUT do
      xSpeed = xSpeed + cosAngle
      ySpeed = ySpeed + sinAngle
      xEnd = xEnd + xSpeed
      yEnd = yEnd + ySpeed
      if (outOfBounds(xEnd, yEnd)) then
        break
      end
    end
    if (not outOfBounds(xEnd, yEnd)) then
      local risk = evalRisk(xStart, yStart, xEnd, yEnd,
          math.atan2(yEnd - yStart, xEnd - xStart))
      if (risk < bestRisk) then
        bestRisk = risk
        bestAngle = angle
      end
    end
  end
  
  ship:fireThruster(bestAngle, 1)
end

function evalRisk(xStart, yStart, xDest, yDest, angle)
  local risk = 0
  for i, enemyShip in pairs(shipData) do
    if (enemyShip.alive) then
      risk = risk + (enemyShip.energy
          * (1 + math.abs(math.cos(angle - math.atan2(yDest - enemyShip.y,
                                             xDest - enemyShip.x))))
          / distSq(enemyShip.x, enemyShip.y, xDest, yDest))
    end
  end
  if (risk == 0) then risk = 1 end
  risk = risk / (100 + math.min(500, distSq(xStart, yStart, xDest, yDest)))
  return risk
end

function fireAtJuiciestTarget()
  if (next(shipData)) then
    local bestRating = math.huge
    local targetShip = nil
    for i, enemyShip in pairs(shipData) do
      if (enemyShip.alive) then
        local rating = enemyShip.energy
            * distSq(ship:x(), ship:y(), enemyShip.x, enemyShip.y)
        if (rating < bestRating) then
          targetShip = enemyShip
          bestRating = rating
        end
      end
    end

    if (targetShip ~= nil) then
      local distance = math.sqrt(
          distSq(ship:x(), ship:y(), targetShip.x, targetShip.y))
      local laserFlightTime =
          math.ceil((distance - constants.LASER_SPEED) / constants.LASER_SPEED)
      local randomizer = (math.random() * .8) + 0.4
      local shipRadius = constants.SHIP_RADIUS
      local targetx = math.max(shipRadius, math.min(world:width() - shipRadius,
          targetShip.x
              + (math.cos(targetShip.heading) * targetShip.speed
                  * laserFlightTime * randomizer)))
      local targety = math.max(shipRadius, math.min(world:height() - shipRadius,
          targetShip.y
              + (math.sin(targetShip.heading) * targetShip.speed
                  * laserFlightTime * randomizer)))
      local laserAngle = math.atan2(targety - ship:y(), targetx - ship:x())
      ship:fireLaser(laserAngle)
      ship:fireTorpedo(math.atan2(
          targetShip.y - ship:y(), targetShip.x - ship:x()), distance)
    end
  end
end

function square(x)
  return x * x
end

function outOfBounds(x, y)
  local shipRadius = constants.SHIP_RADIUS
  return (x < shipRadius or y < shipRadius or x > worldWidth - shipRadius
      or y > worldHeight - shipRadius)
end

function distSq(x1, y1, x2, y2)
  return square(x1 - x2) + square(y1 - y2)
end
