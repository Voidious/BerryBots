-- A single player maze with a few walls. Ship starts in bottom left corner and
-- needs to get to top left corner.
--
-- Sample ships that can solve this maze: Maze1bot, RandomBot, WallHugger,
-- and Snail.

require "samplestage"

function configure(stageBuilder)
  stageBuilder:setSize(1000, 700)
  stageBuilder:addStart(50, 50)
  stageBuilder:addWall(248, 0, 4, 200)
  stageBuilder:addWall(648, 148, 4, 200)
  stageBuilder:addWall(0, 348, 850, 4)
  stageBuilder:addWall(648, 500, 4, 200)
  stageBuilder:addWall(248, 352, 4, 200)
  stageBuilder:addZone(0, 650, 50, 50)
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
  samplestage.checkSinglePlayer(ships, admin)

  for i, ship in pairs(ships) do
    if (ship:alive() and world:touchedAnyZone(ship)) then
      admin:setWinner(ship)
      admin:setStatistic(ship, "Time", world:time())
      admin:gameOver()
      local timeLine = "Time: " .. world:time()
      print(timeLine)
    end
  end
end
