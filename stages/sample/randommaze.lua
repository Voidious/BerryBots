-- A sample maze with a few randomly placed walls. Ship starts in bottom left
-- corner and needs to get to top right corner without hitting any walls. Due
-- to the randomness and lack of constraints on wall placement, it's possible
-- some mazes will not be solvable.
--
-- Snail is the only sample ship that can generally solve the mazes created by
-- this stage.

require "samplestage"

function configure(stageBuilder)
  stageBuilder:setSize(800, 600)
  stageBuilder:addStart(50, 50)
  
  for i = 1,8 do
    x = 100 + math.floor(math.random() * 600)
    y = 100 + math.floor(math.random() * 400)
    width = 100 + math.floor(math.random() * 100)
    height = 2
    if (math.random() > 0.5) then
      width, height = height, width
    end
    x = x - (width / 2)
    y = y - (height / 2)
    stageBuilder:addWall(x, y, width, height)
  end
  
  stageBuilder:addZone(600, 500, 50, 50)
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
    elseif (ship:hitWall()) then
      admin:setStatistic(ship, "Time", world:time())
      admin:gameOver()
    end
  end
end
