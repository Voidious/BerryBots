-- A "fixed shooter" arcade style game, sort of like Galaga or Space Invaders.
-- Player ship can only move from side to side along the bottom of the stage and
-- gains points by destroying the drone ships.
--
-- Shooter is the only sample ship designed for this stage, but any ship that
-- moves and fires will also work.

function configure(stageBuilder)
  stageBuilder:setSize(600, 700)
  stageBuilder:setBattleMode(true)
  stageBuilder:addStart(300, 50)
  for i=1,8 do
    stageBuilder:addShip("shooterdrone.lua")
  end
end

world = nil
admin = nil
userShips = { }
stageShips = { }
shipsByName = { }

function init(shipsArg, worldArg, adminArg)
  local ships = shipsArg
  world = worldArg
  admin = adminArg

  for i, ship in pairs(ships) do
    if (ship:isStageShip()) then
      table.insert(stageShips, ship)
      admin:setShipShowName(ship, false)
    else
      table.insert(userShips, ship)
    end
    shipsByName[ship:name()] = ship
  end
  reviveAllShips()
end

score = 0
messageTimer = 0
message = nil

function run(stageSensors)
  if (userShipsAlive() == 0) then
    print ("Final score: " .. score)
    for i, ship in pairs(userShips) do
      admin:setScore(ship, score)
      admin:setStatistic(ship, "Survival time", world:time())
    end
    admin:gameOver()
  end

  for i, ship in pairs(userShips) do
    admin:moveShip(ship, ship:x(), 50)
    local cosHeading = math.cos(ship:heading())
    admin:setShipSpeed(ship, ship:speed() * math.abs(cosHeading))
    if (cosHeading > 0) then
      admin:setShipHeading(ship, 0)
    else
      admin:setShipHeading(ship, math.pi)
    end
  end

  for i, sde in pairs(stageSensors:shipDestroyedEvents()) do
    if (shipsByName[sde.shipName]:isStageShip()) then
      score = score + 1
    end
  end

  if (stageShipsAlive() == 0) then
    messageTimer = 180
    message = "+10 for killing all drones!"
    score = score + 10
    reviveAllShips()
  end

  if (messageTimer > 0) then
    admin:drawText(message, 10, 630)
    messageTimer = messageTimer - 1
  end

  admin:drawText("Score: " .. score, 10, 670)
end

function stageShipsAlive()
  return shipsAlive(stageShips)
end

function userShipsAlive()
  return shipsAlive(userShips)
end

function shipsAlive(ships)
  local shipsAlive = 0
  for i, ship in pairs(ships) do
    if (ship:alive()) then
      shipsAlive = shipsAlive + 1
    end
  end
  return shipsAlive
end

function reviveAllShips()
  -- First, move them all out of the way. If we try to place a ship onto an
  -- existing ship, the engine is smart enough to choose a valid nearby position
  -- instead. But we're moving all of them, so it's fine to do this first so
  -- they end up exactly where we want.
  for i, ship in pairs(stageShips) do
    admin:moveShip(ship, i * 25, 350)
  end
  
  for i, ship in pairs(stageShips) do
    admin:reviveShip(ship)
    admin:setShipSpeed(ship, 0)
    local formation = { }

    -- orientation: 1 = start on left half of screen, -1 on right side
    -- offset: Ticks to thruster towards reference point in top middle middle
    --         at the beginning of the pattern. With distance of 60 between
    --         ships, it would take 12 ticks at full speed. Add 2 more ticks
    --         for first gap (1/2/3/4 vs 5/5) and another 2 for last gap
    --         (4/3/2/1 vs 5/5), and 1 more to come to complete stop.
    -- startTime: time this movement pattern starts (next tick)
    if (i <= 4) then
      admin:moveShip(ship, 100 + ((i - 1) * 60), 650)
      formation.orientation = 1
      if (i == 4) then
        formation.offset = 5
      elseif (i == 3) then
        formation.offset = 17
      else
        formation.offset = 17 + ((3 - i) * 12)
      end
    else
      admin:moveShip(ship, 320 + ((i - 5) * 60), 650)
      formation.orientation = -1
      if (i == 5) then
        formation.offset = 5
      elseif (i == 6) then
        formation.offset = 17
      else
        formation.offset = 17 + ((i - 6) * 12)
      end
    end
    formation.startTime = world:time() + 1
    admin:sendEvent(ship, formation)
  end
end
