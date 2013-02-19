-- A simple racing ship designed for the sample.racetrack stage. Has a hard
-- coded path and no intelligence for quick adaptation to collisions or finding
-- optimal paths.

ship = nil
world = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  ship:setName("Racer")
  ship:setShipColor(255, 0, 0)
  ship:setLaserColor(255, 0, 0)
  ship:setThrusterColor(255, 255, 0)
end

lastHitWall = false
lastHitShip = false

function run(enemyShips, sensors)

  if ((lastHitWall and ship:hitWall())
      or (lastHitShip and ship:hitShip())) then
    ship:fireThruster(math.random() * 2 * math.pi, 5 - ship:speed())
  else
    if (ship:x() < 750 and ship:y() < 200) then
      shipGoto(850, 150)
    elseif (ship:x() > 750 and ship:y() < 450) then
      shipGoto(925, 550)
    elseif (ship:x() > 200) then
      shipGoto(75, 600)
    else
      shipGoto(75, 150)
    end
  end
  lastHitWall = ship:hitWall()
  lastHitShip = ship:hitShip()
end

function shipGoto(x, y)
  angle = math.atan2(y - ship:y(), x - ship:x())
  ship:fireThruster(angle, 12 - ship:speed())
end
