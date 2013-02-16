-- A ship designed to play the sample.arcadeshooter stage. Moves back and forth
-- at a steady rate and shoots directly at the drone with some combination of
-- low energy and nearby location.

ship = nil
world = nil
constants = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  constants = world:constants()
  ship:setName("Shooter")
end

direction = math.pi

function run(enemyShips, sensors)
  if (# sensors:hitWallEvents() > 0) then
    direction = (direction + math.pi) % (2 * math.pi)
  end
  ship:fireThruster(direction, 10 - ship:speed())

  fireAtJuiciestTarget(enemyShips)
end

function fireAtJuiciestTarget(enemyShips)
  if (# enemyShips > 0) then
    local bestRating = math.huge
    local targetShip = nil
    for i, enemyShip in pairs(enemyShips) do
      local rating = enemyShip.energy
          * square(ship:x() - enemyShip.x) + square(ship:y() - enemyShip.y)
      if (rating < bestRating) then
        targetShip = enemyShip
        bestRating = rating
      end
    end
    local distance = math.sqrt(
        distSq(ship:x(), ship:y(), targetShip.x, targetShip.y))

    local laserFlightTime =
        math.ceil((distance - constants.LASER_SPEED) / constants.LASER_SPEED)
    local randomizer = (math.random() * .8) + 0.4
    local targetx = math.max(8, math.min(world:width() - 8,
        targetShip.x
            + (math.cos(targetShip.heading) * targetShip.speed
                * laserFlightTime * randomizer)))
    local targety = math.max(8, math.min(world:height() - 8,
        targetShip.y
            + (math.sin(targetShip.heading) * targetShip.speed
                * laserFlightTime * randomizer)))
    local laserAngle = math.atan2(targety - ship:y(), targetx - ship:x())
    ship:fireLaser(laserAngle)

--    Be careful firing torpedos! Those drones might not stay at a safe distance
--    if you start knocking them off their usual path. =)
--
--    if (distance > constants.TORPEDO_BLAST_RADIUS) then
--      local torpedoFlightTime = math.ceil((distance - constants.SHIP_RADIUS) / constants.TORPEDO_SPEED)
--      local torpedoAngle =
--          math.atan2(targetShip.y - ship:y(), targetShip.x - ship:x())
--      ship:fireTorpedo(torpedoAngle, distance)
--    end
  end
end

function square(x)
  return x * x
end

function distSq(x1, y1, x2, y2)
  return square(x1 - x2) + square(y1 - y2)
end
