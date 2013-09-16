-- A basic battle field with some walls.
--
-- Sample battle ships: Chaser, FloatingDuck, MyFirstShip, RandomBot,
-- WallHugger, and BasicBattler.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(1200, 800)
  stageBuilder:setBattleMode(true)
  stageBuilder:addWall(200, 296, 300, 4)
  stageBuilder:addWall(496, 150, 4, 146)
  stageBuilder:addWall(700, 296, 300, 4)
  stageBuilder:addWall(700, 150, 4, 146)
  stageBuilder:addWall(200, 500, 300, 4)
  stageBuilder:addWall(496, 504, 4, 146)
  stageBuilder:addWall(700, 500, 300, 4)
  stageBuilder:addWall(700, 504, 4, 146)
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
