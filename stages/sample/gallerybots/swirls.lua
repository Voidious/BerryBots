-- A target ship for the sample.lasergallery targeting challenge stage.
-- Moves in big arcs without getting too close.

MAX_SPEED = 12

ship = nil
world = nil
constants = nil
orientation = 1

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  constants = world:constants()
  ship:setName("Swirls")
  ship:setShipColor(80, 80, 80)
  ship:setLaserColor(80, 80, 80)
  ship:setThrusterColor(80, 80, 80)
end

function run(enemyShips, sensors)
  if (ship:hitWall() and math.random() < 0.5) then
    orientation = orientation * -1
  end
  if (ship:x() < 650) then
    ship:fireThruster(0, 1)
    if (math.random() < 0.5) then
      orientation = orientation * -1
    end
  elseif (ship:speed() < 0.0001) then
    ship:fireThruster(math.random() * 2 * math.pi, 1)
  elseif (ship:speed() < MAX_SPEED - 0.5) then
    ship:fireThruster(ship:heading(), MAX_SPEED - ship:speed())
  else
    ship:fireThruster(ship:heading() + (math.pi * orientation * .5), 1)
  end
end
