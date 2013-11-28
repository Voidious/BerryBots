-- A ship control program.

ship = nil
world = nil
heading = nil
force = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg

  ship:setName("Player")
  ship:setLaserColor(255, 255, 0)
  heading = 0
  force = world:constants().MAX_THRUSTER_FORCE
end

function run(enemyShips, sensors)
  ship:fireThruster(heading, force)
end

function roundOver()

end

function gameOver()

end
