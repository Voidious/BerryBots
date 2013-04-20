-- A stage for 2-ship jousting battles.
--
-- Ships start on opposite sides of the stage and start the battle moving at
-- high speed towards each other. When a ship is knocked into either of the
-- zones to the side of the center battle strip, it is disqualified and the
-- other ship wins.
--
-- Sample ships that can joust: Jouster and Chaser.

require "samplestage"

local TIME_LIMIT = 5000

function configure(stageBuilder)
  stageBuilder:setSize(1000, 700)
  stageBuilder:addStart(50, 350 + (20 * math.random()) - 10)
  stageBuilder:addStart(950, 350 + (20 * math.random()) - 10)
  stageBuilder:addWall(0, 246, 100, 4)
  stageBuilder:addWall(0, 450, 100, 4)
  stageBuilder:addWall(900, 246, 100, 4)
  stageBuilder:addWall(900, 450, 100, 4)
  stageBuilder:addZone(0, 0, 1000, 250)
  stageBuilder:addZone(0, 450, 1000, 250)
end

ships = nil
world = nil
admin = nil

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg
end

function run()
  samplestage.checkNumPlayers(2, ships, admin)

  if (world:time() == 1) then
    for i, ship in pairs(ships) do
      admin:setShipSpeed(ship, 20)
      if (ship:x() < 600) then
        admin:setShipHeading(ship, 0)
      else
        admin:setShipHeading(ship, math.pi)
      end
    end
  end

  shipsAlive = 0
  for i, ship in pairs(ships) do
    if (world:inAnyZone(ship)) then
      admin:destroyShip(ship)
    end
    if (ship:alive()) then
      shipsAlive = shipsAlive + 1
    end
  end

  if (shipsAlive <= 1) then
    for i, ship in pairs(ships) do
      if (ship:alive()) then
        admin:setWinner(ship)
      end
    end
    admin:gameOver()
  end

  if (world:time() >= TIME_LIMIT) then
    admin:gameOver()
  end
end
