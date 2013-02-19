-- A jousting ship. Starts the battle by strafing to the side to avoid the
-- initial collision, then tries to stay away from the dangerous edges of the
-- stage while otherwise move directly at the enemy.

ship = nil
world = nil
width = nil
height = nil

function init(shipRef, worldArg)
  ship = shipRef
  world = worldArg
  width = world:width()
  height = world:height()
  ship:setName("Jouster")
  ship:setShipColor(255, 0, 0)
  ship:setLaserColor(100, 100, 100)
  ship:setThrusterColor(255, 0, 0)
end

above = nil

function run(enemyShips)
  local enemyShip = enemyShips[1]
  if (below == nil) then
    if (ship:y() < enemyShip.y) then
      below = 1
    else
      below = -1
    end
  end

  if (world:time() <= 8) then
    ship:fireThruster((math.pi / 2) * -below, 1)
  elseif (world:time() <= 16) then
    ship:fireThruster((math.pi / 2) * below, 1)
  else
    local upwards
    local saving = false
    if (ship:speed() > 5) then
      local sinHeading = math.sin(ship:heading())
      local yOffset = ship:y() - (world:height() / 2)
      if ((sinHeading > 0.2 and yOffset > 25) or yOffset > 50) then
        saving = true
        upwards = -1
      elseif ((sinHeading < -0.2 and yOffset < -25) or yOffset < -50) then
        saving = true
        upwards = 1
      end
    end
    if (saving) then
      ship:fireThruster((math.pi / 2) * upwards, 1)
    else
      angleToEnemy =
          math.atan2(enemyShip.y - ship:y(), enemyShip.x - ship:x())
      ship:fireThruster(angleToEnemy, 1)
    end
  end
end
