-- A basic battle field with a few randomly placed / sized walls.
--
-- Sample battle ships: Chaser, FloatingDuck, MyFirstShip, RandomBot,
-- WallHugger, and BasicBattler.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(1200, 800)
  stageBuilder:setBattleMode(true)
  for i = 1,3 do
    x = 200 + math.floor(math.random() * 800)
    y = 100 + math.floor(math.random() * 800)
    width = 400 + math.floor(math.random() * 400)
    height = 2
    if (math.random() > 0.5) then
      width, height = height, width
    end
    x = x - (width / 2)
    height = height - (height / 2)
    stageBuilder:addWall(x, y, width, height)
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
