-- A target ship for the sample.lasergallery targeting challenge stage.
-- Moves randomly in long straight lines.

MAX_SPEED = 15

ship = nil
world = nil
destination = nil

function init(shipsArg, worldArg)
  ship = shipsArg
  world = worldArg

  ship:setName("Lines")
  ship:setShipColor(150, 0, 175)
  ship:setLaserColor(150, 0, 175)
  ship:setThrusterColor(150, 0, 175)
end

function run(enemyShips, sensors)
  if (ship:speed() < 0.0001 or destination == nil) then
    destination = {x = 650 + (math.random() * 484),
                   y = 16 + (math.random() * 768)}
  end

  local distanceToDestination = math.sqrt(square(destination.x - ship:x())
      + square(destination.y - ship:y()))
  local maxSpeed = math.min(MAX_SPEED, getMaxVelocity(distanceToDestination))
  local bearingToDestination =
      math.atan2(destination.y - ship:y(), destination.x - ship:x())

  if (distanceToDestination < 1) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  else
    local goBearing
    local force
    if (ship:speed() > maxSpeed) then
      goBearing = bearingToDestination + math.pi
      force = ship:speed() - maxSpeed
    else
      goBearing = bearingToDestination
      force = maxSpeed - ship:speed()
    end
    ship:fireThruster(goBearing, force)
  end
end

function getMaxVelocity(distance)
  local decelTime = decelTime(distance)
  local decelDist = (decelTime / 2.0) * (decelTime - 1) -- sum of 0..(decelTime-1)
  return (decelTime - 1) + ((distance - decelDist) / decelTime)
end

function decelTime(distance)
  local x = 1
  while (true) do
    -- (square(x) + x) / 2) = 1, 3, 6, 10, 15...
    if (distance <= ((square(x) + x) / 2)) then
      return x
    end
    x = x + 1
  end
  return -1
end

function square(x)
  return x * x
end
