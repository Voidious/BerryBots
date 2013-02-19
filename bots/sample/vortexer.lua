-- A ship designed to play the sample.vortex stage. Tries to get as far away
-- from the center as possible until it starts getting sucked in, then fires its
-- thruster perpendicular to try and slingshot around the center.
--
-- Its strategy is focused only on surviving, not actually aiming at the target
-- zones.

ship = nil
world = nil
center = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  center = {x = (world:width() / 2), y = (world:height() / 2)}
  ship:setName("Vortexer")
  ship:setShipColor(255, 0, 255)
  ship:setThrusterColor(255, 255, 255)
end

function square(x)
  return x * x
end

slingshotDirection =  1
hitWall = false

function run(enemyShips, sensors)
  local bearingToCenter =
      math.atan2(center.y - ship:y(), center.x - ship:x())
  local distanceToCenter =
      math.sqrt(square(center.x - ship:x()) + square(center.y - ship:y()))
  local stuckOnWall = false

  if (ship:hitWall()) then
    if (hitWall) then
      stuckOnWall = true
    end
    hitWall = true
  end

  if (stuckOnWall) then
    ship:fireThruster(math.random() * 2 * math.pi, 1)
  else
    if (math.cos(ship:heading() - bearingToCenter) < 0) then
      ship:fireThruster(bearingToCenter + math.pi, 1)
      slingshotDirection = slingshotDirection * -1
    else
      ship:fireThruster(bearingToCenter + (math.pi * slingshotDirection / 2), 1)
    end
  end
end
