-- Drone ship loaded and managed by the sample.drift stage.

ship = nil
world = nil

function init(shipRef, worldArg)
  ship = shipRef
  world = worldArg
  ship:setName("Drone")
  ship:setShipColor(0, 0, 255)
  ship:setLaserColor(255, 255, 255)
  ship:setThrusterColor(0, 0, 0)
end

direction = math.pi
speed = 10

function run(enemyShips, sensors)
  for i, se in pairs(sensors:stageEvents()) do
    speed = se.speed
    direction = se.direction
  end

  if (# sensors:hitWallEvents() > 0) then
    direction = (direction + math.pi) % (2 * math.pi)
  end
  ship:fireThruster(direction, speed - ship:speed())
end
