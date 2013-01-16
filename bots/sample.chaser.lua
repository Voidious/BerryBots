-- A simple bot that moves directly at any bot it sees and fires at them. Just
-- moves back and forth if it doesn't see anyone.

ship = nil
world = nil
targetName = nil

function init(shipRef, worldArg)
  ship = shipRef
  world = worldArg
  ship:setName("Chaser")
end

angle = math.random() * 2 * math.pi
function run(enemyShips)
  local time = world:time()
  local targetx, targety = nil, nil
  if (# enemyShips == 0) then
    targetName = nil
  else
    if (targetName == nil or not seeTargetShip(enemyShips, targetName)) then
      targetName = enemyShips[1].name
    end
    for i, enemyShip in pairs(enemyShips) do
      if (targetName == enemyShip.name) then
        targetx = enemyShip.x
        targety = enemyShip.y
      end
    end
  end

  if (targetx == nil) then
    if (time % 12 == 0) then
        angle = (angle + math.pi) % (2 * math.pi)
    end
    if (time % 12 < 6 and ship:speed() > 0) then
      ship:fireThruster(ship:heading() + math.pi, ship:speed())
    else
      ship:fireThruster(angle, 1)
    end
  else
    shipGoto(targetx, targety)
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

function shipGoto(x, y)
  angle = math.atan2(y - ship:y(), x - ship:x())
  ship:fireThruster(angle, 1)
end

function square(x)
  return x * x
end

function distance(x1, y1, x2, y2)
  return math.sqrt(square(x1 - x2) + square(y1 - y2))
end
