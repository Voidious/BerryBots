-- A basic battle field with some walls.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(1200, 800)
  stageBuilder:setBattleMode(true)
  stageBuilder:addWall(300, 225, 10, 350)
  stageBuilder:addWall(895, 225, 10, 350)
end

ships = nil
admin = nil

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  admin = adminArg
end

function run(stageSensors)
  battlestage.basicScoring(ships, admin, 437, 770, 16)
end
