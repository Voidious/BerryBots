-- The drone ship loaded and managed by sample.arcadeshooter stage. Fires lasers
-- at long intervals and moves in a pattern, oriented by the stage to stay in
-- formation with other ships.

ship = nil
world = nil
constants = nil
fireOffset = nil
direction = nil
fireInterval = 25

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
  constants = world:constants()
  ship:setName("Drone")
  ship:setShipColor(255, 0, 0)
  ship:setLaserColor(255, 0, 0)
  ship:setThrusterColor(200, 200, 200)

  fireOffset = math.floor(math.random() * fireInterval)
  direction = math.pi * .5
  if (math.random() > 0.5) then
    direction = direction + math.pi
  end
end

orientation = 1
offset = 0
startTime = 0

function run(enemyShips, sensors)
  for i, se in pairs(sensors:stageEvents()) do
    orientation = se.orientation
    offset = se.offset
    startTime = se.startTime
  end

  local firingAngle = math.pi * 1.5
  for i, enemyShip in pairs(enemyShips) do
    if (not enemyShip.isStageShip) then
      firingAngle = math.atan2(enemyShip.y - ship:y(), enemyShip.x - ship:x())
    end
  end

  local time = world:time()
  local refTime = startTime + offset
  local cycleSize = 260
  if (time < refTime - 5) then
    ship:fireThruster(leftOrRight(orientation), 5 - ship:speed())
  elseif (time < refTime) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  elseif (((time - refTime) % cycleSize) < 80) then
    ship:fireThruster(math.pi * 1.5, 5 - ship:speed())
  elseif (((time - refTime) % cycleSize) < 85) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  elseif ((time - refTime) % cycleSize < 125) then
    ship:fireThruster(leftOrRight(-1 * orientation), 5 - ship:speed())
  elseif ((time - refTime) % cycleSize < 130) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  elseif ((time - refTime) % cycleSize < 210) then
    ship:fireThruster(math.pi / 2, 5 - ship:speed())
  elseif ((time - refTime) % cycleSize < 215) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  elseif ((time - refTime) % cycleSize < 255) then
    ship:fireThruster(leftOrRight(orientation), 5 - ship:speed())
  elseif ((time - refTime) % cycleSize < 260) then
    ship:fireThruster(ship:heading() + math.pi, ship:speed())
  end

  if ((world:time() + fireOffset) % fireInterval == 0) then
    ship:fireLaser(firingAngle)
  end
end

function leftOrRight(orientation)
  if (orientation > 0) then
    return 0
  else
    return math.pi
  end
end
