-- A sample maze with a few walls. Ship starts in bottom left corner
-- and needs to get to top left corner.

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
  if (# ships > 1) then
    admin:drawText("WARNING: This stage is only designed for 1 ship.", 20, 12)
  end
  for i,ship in pairs(ships) do
    if (world:touchedAnyZone(ship)) then
      admin:setWinner(ship:name())
      admin:gameOver()
      local timeLine = "Time: " .. world:time()
      print(timeLine)
      admin:drawText(timeLine, 430, 650, 32)
    end
  end
end
