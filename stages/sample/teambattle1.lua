-- A 5v5 team battle stage.

require "battlestage"

function configure(stageBuilder)
  stageBuilder:setSize(2400, 1600)
  stageBuilder:setBattleMode(true)
  stageBuilder:addWall(250, 200, 100, 100)
  stageBuilder:addWall(2150, 200, 100, 100)
  stageBuilder:addWall(250, 1300, 100, 100)
  stageBuilder:addWall(2150, 1300, 100, 100)
  stageBuilder:addWall(1150, 750, 100, 100)
  stageBuilder:setTeamSize(5)
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
  battlestage.basicScoring(ships, world, admin, 874, 1540, 32)
end
