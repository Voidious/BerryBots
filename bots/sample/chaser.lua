-- A battle ship that moves directly at any ship it sees and fires at it. If
-- Chaser doesn't see anyone, it just moves back and forth.
--
-- This also works as a basic strategy for sample.joust. Can also be initialized
-- as a team of any size, controlling each ship with the same logic and totally
-- ignoring teammates.

ships = { }
shipStates = { }
world = nil

function init(shipsArg, worldArg)
  if (type(shipsArg) == "userdata") then
    table.insert(ships, shipsArg)
  else
    ships = shipsArg
    ships[1]:setTeamName("ChaserTeam")
  end
  world = worldArg

  for i, ship in pairs(ships) do
    ship:setName("Chaser")
    ship:setShipColor(0, 0, 255)
    ship:setLaserColor(255, 255, 255)
    ship:setThrusterColor(255, 255, 255)
    local shipState = {angle=math.random() * 2 * math.pi, targetName = nil}
    shipStates[ship] = shipState
  end
end

function run(enemyShips)
  for i, ship in pairs(ships) do
    runShip(ship, enemyShips)
  end
end

function runShip(ship, enemyShips)
  local state = shipStates[ship]
  local time = world:time()
  local targetx, targety = nil, nil
  if (# enemyShips == 0) then
    state.targetName = nil
  else
    if (state.targetName == nil
        or not seeTargetShip(enemyShips, state.targetName)) then
      state.targetName = enemyShips[1].name
    end
    for i, enemyShip in pairs(enemyShips) do
      if (state.targetName == enemyShip.name) then
        targetx = enemyShip.x
        targety = enemyShip.y
      end
    end
  end

  if (targetx == nil) then
    if (time % 12 == 0) then
        state.angle = (state.angle + math.pi) % (2 * math.pi)
    end
    if (time % 12 < 6 and ship:speed() > 0) then
      ship:fireThruster(ship:heading() + math.pi, ship:speed())
    else
      ship:fireThruster(state.angle, 1)
    end
  else
    shipGoto(ship, targetx, targety)
    local targetDistance = distance(targetx, targety, ship:x(), ship:y())
    local firingAngle = math.atan2(targety - ship:y(), targetx - ship:x())
    ship:fireLaser(firingAngle)
    ship:fireTorpedo(firingAngle, targetDistance)
  end
end

function seeTargetShip(enemyShips, targetName)
  for i, enemyShip in pairs(enemyShips) do
    if (enemyShip.name == targetName) then
      return true
    end
  end
  return false
end

function shipGoto(ship, x, y)
  local angle = math.atan2(y - ship:y(), x - ship:x())
  ship:fireThruster(angle, 1)
end

function square(x)
  return x * x
end

function distance(x1, y1, x2, y2)
  return math.sqrt(square(x1 - x2) + square(y1 - y2))
end
