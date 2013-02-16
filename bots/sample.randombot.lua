-- A battle ship that moves around randomly, tries not to get stuck on walls,
-- and shoots at a random enemy. Can also be initialized as a team of any size,
-- controlling each ship with the same logic and totally ignoring teammates.

ships = { }
dests = { }
thrusts = { }
width = nil
height = nil
world = nil

function init(shipsArg, worldArg)
  if (type(shipsArg) == "userdata") then
    table.insert(ships, shipsArg)
  else
    ships = shipsArg
    ships[1]:setTeamName("RandomTeam")
  end
  world = worldArg
  width = world:width()
  height = world:height()
  for i, ship in pairs(ships) do
    ship:setName("RandomBot")
  end
end

function setRandomDestination(ship)
  if (dests[ship] == nil) then
    dests[ship] = { }
  end
  dests[ship].x = math.random() * width
  dests[ship].y = math.random() * height
end

function distanceToDestination(ship)
  return math.sqrt(math.pow(dests[ship].x - ship:x(), 2)
      + math.pow(dests[ship].y - ship:y(), 2))
end

function square(x)
  return x * x
end

function distance(x1, y1, x2, y2)
  return math.sqrt(square(x1 - x2) + square(y1 - y2))
end

function run(enemyShips, sensors)
  for i, ship in pairs(ships) do
    runShip(ship, enemyShips, sensors)
  end
end

function runShip(ship, enemyShips, sensors)
  local time = world:time()
  local dest = dests[ship]
  if (dest == nil or distanceToDestination(ship) < 150 or time % 50 == 0
      or ship:hitWall() or ship:hitShip()) then
    setRandomDestination(ship)
    thrusts[ship] = .1 + math.random()
    dest = dests[ship]
  end
  shipGoto(ship, dest.x, dest.y, thrust)
  local laserAngle = math.random() * math.pi * 2
  if (# enemyShips > 0) then
    local targetShip =
        enemyShips[math.floor(math.random() * (# enemyShips)) + 1]
    local targetDistance =
        distance(targetShip.x, targetShip.y, ship:x(), ship:y())
    local firingAngle = math.atan2(targetShip.y - ship:y(),
                                   targetShip.x - ship:x())
    ship:fireLaser(firingAngle)
    ship:fireTorpedo(firingAngle, targetDistance)
  end
end

function shipGoto(ship, x, y)
  local angle = math.atan2(y - ship:y(), x - ship:x())
  ship:fireThruster(angle, thrusts[ship])
end
