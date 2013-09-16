-- A massive battle field.
--
-- Sample battle ships: Chaser, FloatingDuck, MyFirstShip, RandomBot,
-- WallHugger, and BasicBattler.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(6400, 4000)
  stageBuilder:setBattleMode(true)

  for i = 1, 7 do
    for j = 1, 5 do
      if (i % 2 ~= j % 2) then
        local wallCenter = {x = (i * 800), y = (j * 800) - 400}
        stageBuilder:addWall(wallCenter.x - 100, wallCenter.y - 100, 200, 200)
      end
    end
  end
end

ships = nil
world = nil
admin = nil

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg
end

function run(stageSensors)
  battlestage.basicScoring(ships, world, admin)
end
