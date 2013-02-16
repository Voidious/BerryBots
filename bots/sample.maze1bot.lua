-- A ship designed to navigate sample.maze1.lua. Path is developed through trial
-- and error, hard coded and not doing anything intelligent.

ship = nil
world = nil

function init(shipArg, worldArg)
  ship = shipArg
  world = worldArg
end

function run()
  time = world:time()
  if (time <= 10) then
    ship:fireThruster(math.pi * .25, 1)
  elseif (time > 35 and time <= 45) then
    ship:fireThruster(math.pi * 1.25, 1)
  elseif (time > 45 and time <= 55) then
    ship:fireThruster(math.pi * 1.85, 1)
  elseif (time > 100 and time <= 110) then
    ship:fireThruster(math.pi * .5, 1)
  elseif (time > 120 and time <= 130) then
    ship:fireThruster(math.pi * .85, 1)
  elseif (time > 133 and time <= 143) then
    ship:fireThruster(math.pi, 1)
  elseif (time > 143 and time <= 153) then
    ship:fireThruster(math.pi * 1.5, 1)
  elseif (time > 170 and time <= 180) then
    ship:fireThruster(math.pi * .5, 1)
  elseif (time > 200 and time <= 210) then
    ship:fireThruster(math.pi * 1.5, 1)
  end
end
