-- A target ship for the sample.lasergallery targeting challenge stage.
-- Moves back and forth in a small range.

ship = nil
world = nil
angle = math.random() * 2 * math.pi

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  ship:setName("Paces")
  ship:setShipColor(255, 255, 255)
  ship:setLaserColor(255, 255, 255)
  ship:setThrusterColor(255, 255, 255)
end

function run(enemyShips)
  time = world:time()

  if (time % 20 == 0) then
    angle = (angle + math.pi) % (2 * math.pi)
  end
  if (time % 20 < 10) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  else
    ship:fireThruster(angle, 1)
  end
end
