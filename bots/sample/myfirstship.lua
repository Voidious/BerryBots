-- A battle ship that just moves back and forth and shoots at any enemy it sees.

ship = nil
world = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  ship:setName("MyFirstShip")
end

angle = math.random() * 2 * math.pi
function run(enemyShips)
  time = world:time()

  if (time % 20 == 0) then
      angle = (angle + math.pi) % (2 * math.pi)
  end
  if (time % 20 < 10 and ship:speed() > 0) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  else
    ship:fireThruster(angle, 1)
  end

  if (# enemyShips > 0) then
    targetShip = enemyShips[math.floor(math.random() * (# enemyShips)) + 1]
    targetDistance = distance(targetShip.x, targetShip.y, ship:x(), ship:y())
    firingAngle = math.atan2(targetShip.y - ship:y(),
                             targetShip.x - ship:x())
    ship:fireLaser(firingAngle)
    ship:fireTorpedo(firingAngle, targetDistance)
  end
end

function square(x)
  return x * x
end

function distance(x1, y1, x2, y2)
  return math.sqrt(square(x1 - x2) + square(y1 - y2))
end
