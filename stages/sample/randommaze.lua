-- A sample maze with a few randomly placed walls. Bot starts in bottom left
-- corner and needs to get to top right corner without hitting any walls. Due
-- to the randomness and lack of constraints on wall placement, it's possible
-- some mazes will not be solvable.

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
  if (# ships > 1) then
    admin:drawText("WARNING: This stage is only designed for 1 ship.", 20, 26)
  end
  for i,ship in pairs(ships) do
    if (world:touchedAnyZone(ship)) then
      admin:setWinner(ship:name())
      admin:gameOver()
    elseif (ship:hitWall()) then
      admin:gameOver()
    end
  end
end
