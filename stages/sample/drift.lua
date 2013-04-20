-- A movement only, single player stage. The goal is to get to the other side of
-- the stage without hitting any enemy ships or walls. Movement is restricted to
-- a single thruster action before each group of enemy ships, and your ship
-- always moves at a speed of 5.
--
-- Drifter is the only sample ship designed for this stage.

require "samplestage"

DRIFT_SPEED = 5

function configure(stageBuilder)
  stageBuilder:setSize(400, 700)
  stageBuilder:addStart(200, 50)
  for i = 1, 12 do
    stageBuilder:addShip("driftdrone.lua")
    stageBuilder:addStart(100 + (math.random() * 200),
        100 + (math.floor((i - 1) / 3) * 150) + (((i - 1) % 3) * 25))
  end
  stageBuilder:addZone(0, 650, 400, 50)
end

ships = nil
world = nil
admin = nil
userShip = nil
stageShips = { }
shipSettings = { }

prevShipSpeed = 0
firstThreshold = 200
nextThreshold = firstThreshold
tries = 1

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg
  for i, ship in ipairs(ships) do
    if (ship:isStageShip()) then
      local event = { }
      event.speed = math.ceil(math.random() * 18) + 2
      event.direction = 0
      if (math.random() < 0.5) then
        event.direction = math.pi
      end
      admin:sendEvent(ship, event)
      table.insert(stageShips, ship)
      shipSettings[ship:name()] = { }
      shipSettings[ship:name()].y = ship:y()
      shipSettings[ship:name()].speed = event.speed
      admin:reviveShip(ship)
      admin:setShipShowName(ship, false)
    elseif (userShip == nil) then
      userShip = ship
      prevShipSpeed = 0
    end
  end
end

function run(stageSensors)
  samplestage.checkSinglePlayer(ships, admin)

  admin:drawText("Attempt: " .. tries, 15, 12)
  if (userShip:hitWall() or userShip:hitShip()) then
    admin:destroyShip(userShip)
    admin:reviveShip(userShip)
    admin:sendEvent(userShip, {eventType="restart"})
    admin:moveShip(userShip, 200, 50)
    admin:setShipSpeed(userShip, 0)
    admin:setShipThrusterEnabled(userShip, true)
    nextThreshold = firstThreshold
    tries = tries + 1
  elseif (userShip:speed() > 0) then
    if (prevShipSpeed == 0) then
      admin:setShipThrusterEnabled(userShip, false)
    end
    if (userShip:y() >= nextThreshold) then
      admin:sendEvent(userShip, {eventType="next"})
      nextThreshold = nextThreshold + 150
      admin:setShipSpeed(userShip, 0)
      admin:setShipThrusterEnabled(userShip, true)
    else
      admin:setShipSpeed(userShip, DRIFT_SPEED)
    end
  end

  prevShipSpeed = userShip:speed()

  for i, ship in pairs(stageShips) do
    if (ship:hitShip()) then
      admin:moveShip(ship, ship:x(), shipSettings[ship:name()].y)
      admin:setShipSpeed(ship, shipSettings[ship:name()].speed)
      local direction = 0
      if (math.random() < 0.5) then
        direction = math.pi
      end
      admin:setShipHeading(ship, direction)
    end
  end

  if (world:touchedAnyZone(userShip)) then
    print("Tries: " .. tries)
    print("Time:  " .. world:time())
    admin:setWinner(userShip)
    admin:setStatistic(userShip, "Tries", tries)
    admin:setStatistic(userShip, "Time", world:time())
    admin:gameOver()
  end
end
