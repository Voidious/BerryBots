-- A reasonably complex single player maze. Ship starts in middle left and needs
-- to get to top right corner without hitting any walls.
--
-- Sample ships that can solve this maze: WallHugger and Snail.

require "samplestage"

function configure(stageBuilder)
  stageBuilder:setSize(1200, 600)
  stageBuilder:addStart(25, 300)
  stageBuilder:addWall(75, 350, 4, 250)
  stageBuilder:addWall(75, 0, 4, 250)
  stageBuilder:addWall(75, 350, 450, 4)
  stageBuilder:addWall(625, 350, 4, 150)
  stageBuilder:addWall(200, 500, 925, 4)
  stageBuilder:addWall(300, 450, 4, 50)
  stageBuilder:addWall(450, 350, 4, 75)
  stageBuilder:addWall(1025, 350, 4, 100)
  stageBuilder:addWall(1125, 500, 4, 100)
  stageBuilder:addWall(625, 350, 500, 4)
  stageBuilder:addWall(1125, 250, 4, 104)
  stageBuilder:addWall(750, 350, 4, 100)
  stageBuilder:addWall(900, 400, 4, 100)
  stageBuilder:addWall(75, 250, 150, 4)
  stageBuilder:addWall(300, 250, 825, 4)
  stageBuilder:addWall(200, 175, 604, 4)
  stageBuilder:addWall(800, 100, 4, 75)
  stageBuilder:addWall(875, 100, 4, 150)
  stageBuilder:addWall(800, 96, 175, 4)
  stageBuilder:addWall(200, 100, 4, 75)
  stageBuilder:addWall(200, 96, 225, 4)
  stageBuilder:addWall(550, 0, 4, 75)
  stageBuilder:addWall(675, 75, 4, 100)
  stageBuilder:addWall(1050, 100, 150, 4)
  stageBuilder:addWall(1050, 175, 4, 75)
  stageBuilder:addWall(875, 175, 175, 4)
  stageBuilder:addZone(1150, 550, 50, 50)
end

world = nil
ships = nil
admin = nil

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg
end

function run()
  samplestage.checkSinglePlayer(ships, admin)

  for i, ship in pairs(ships) do
    if (ship:alive() and world:touchedAnyZone(ship)) then
      admin:setWinner(ship)
      admin:setStatistic(ship, "Time", world:time())
      admin:gameOver()
      local timeLine = "Time: " .. world:time()
      print(timeLine)
    elseif (ship:hitWall()) then
      admin:setStatistic(ship, "Time", world:time())
      admin:gameOver()
    end
  end
end
