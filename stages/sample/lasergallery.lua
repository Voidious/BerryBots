-- A single player targeting challenge. The player ship is stationary and can
-- only shoot its laser gun. A series of ships with different movement styles
-- appear and move around until they are destroyed. The goal is to destroy all
-- ships as quickly as possible.
--
-- BasicBattler is the only sample ship that can hit all five movements.

require "samplestage"

PLAYER_LOCATION = {x = 100, y = 400}
MAX_TIME = 100000

function configure(stageBuilder)
  stageBuilder:setSize(1200, 800)
  stageBuilder:addStart(PLAYER_LOCATION.x, PLAYER_LOCATION.y)
  stageBuilder:addShip("gallerybots/paces.lua")
  stageBuilder:addShip("gallerybots/coasty.lua")
  stageBuilder:addShip("gallerybots/wallsy.lua")
  stageBuilder:addShip("gallerybots/lines.lua")
  stageBuilder:addShip("gallerybots/swirls.lua")
end

ships = nil
userShip = nil
world = nil
admin = nil
stageShips = { }
targetShip = nil
stageShipIndex = 1
numGalleryShips = 0
printShipTimer = 90

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg

  for i, ship in ipairs(shipsArg) do
    if (ship:isStageShip()) then
      table.insert(stageShips, ship)
      admin:setShipEnergyEnabled(ship, true)
      admin:setShipShowName(ship, false)
      numGalleryShips = numGalleryShips + 1
    elseif (userShip == nil) then
      userShip = ship
    end
  end

  admin:setShipLaserEnabled(userShip, true)
  admin:setShipThrusterEnabled(userShip, false)
  admin:setShipTorpedoEnabled(userShip, false)
  admin:setShipEnergyEnabled(userShip, false)

  targetShip = stageShips[1]
  admin:reviveShip(targetShip)
  admin:setShipShowName(targetShip, true)
  local targetLocation = targetStartLocation()
  admin:moveShip(targetShip, targetLocation.x, targetLocation.y)
end

function run(stageSensors)
  samplestage.checkSinglePlayer(ships, admin)

  admin:drawText("Time: " .. world:time(), 20, 15)
  if (printShipTimer > 0) then
    admin:drawText(targetShip:name(), 525, 30, 25)
    printShipTimer = printShipTimer - 1
  end
  admin:moveShip(userShip, PLAYER_LOCATION.x, PLAYER_LOCATION.y)
  if (not targetShip:alive()) then
    stageShipIndex = stageShipIndex + 1
    if (stageShipIndex > numGalleryShips) then
      admin:setWinner(userShip)
      admin:gameOver()
      print("Time: " .. world:time())
      admin:setStatistic(userShip, "Time", world:time())
    else
      admin:setShipShowName(targetShip, false)
      targetShip = stageShips[stageShipIndex]
      admin:roundOver()
      admin:reviveShip(targetShip)
      admin:setShipShowName(targetShip, true)
      local targetLocation = targetStartLocation()
      admin:moveShip(targetShip, targetLocation.x, targetLocation.y)
      printShipTimer = 90
    end
  end
  if (world:time() > MAX_TIME) then
    admin:gameOver()
  end
end

function targetStartLocation()
  return {x = 950 + (math.random() * 100), y = 16 + (math.random() * 768)}
end
