-- A basic battle field with some walls.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(1200, 800)
  stageBuilder:setBattleMode(true)
  stageBuilder:addWall(250, 200, 100, 100)
  stageBuilder:addWall(850, 200, 100, 100)
  stageBuilder:addWall(250, 500, 100, 100)
  stageBuilder:addWall(850, 500, 100, 100)
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
  battlestage.basicScoring(ships, world, admin, 437, 770, 16)
end
