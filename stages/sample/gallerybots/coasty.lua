-- A target ship for the sample.lasergallery targeting challenge stage.
-- Moves up and down at constant speed, reversing direction when it hits a wall.

MAX_SPEED = 10

ship = nil
world = nil
constants = nil
direction = math.pi / 2

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  constants = world:constants()
  ship:setName("Coasty")
  ship:setShipColor(255, 255, 35)
  ship:setLaserColor(255, 255, 35)
  ship:setThrusterColor(255, 255, 35)
end

function run(enemyShips, sensors)
  if (# sensors:hitWallEvents() > 0) then
    direction = (direction + math.pi) % (2 * math.pi)
  end
  ship:fireThruster(direction, MAX_SPEED - ship:speed())
end
