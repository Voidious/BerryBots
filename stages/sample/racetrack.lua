-- An oval race track. Ships need to cross each zone in a counter-clockwise
-- pattern. The ship with the most laps is displayed as the leader on screen,
-- along with the fastest lap. First to 20 laps wins.
--
-- Racer is the only sample ship designed for this stage, but WallHugger can
-- also get around it if he gets lucky and gets onto a counter-clockwise path.

LAPS = 20

function configure(stageBuilder)
  stageBuilder:setSize(1000, 700)

  stageBuilder:addStart(475, 175)
  stageBuilder:addStart(475, 145)
  stageBuilder:addStart(475, 115)
  stageBuilder:addStart(475, 85)
  stageBuilder:addStart(475, 55)
  stageBuilder:addStart(475, 25)
  stageBuilder:addStart(445, 175)
  stageBuilder:addStart(445, 145)
  stageBuilder:addStart(445, 115)
  stageBuilder:addStart(445, 85)
  stageBuilder:addStart(445, 55)
  stageBuilder:addStart(445, 25)
  stageBuilder:addStart(415, 175)
  stageBuilder:addStart(415, 145)
  stageBuilder:addStart(415, 115)
  stageBuilder:addStart(415, 85)
  stageBuilder:addStart(415, 55)
  stageBuilder:addStart(415, 25)

  stageBuilder:addWall(200, 200, 4, 300)
  stageBuilder:addWall(796, 200, 4, 300)
  stageBuilder:addWall(204, 200, 592, 4)
  stageBuilder:addWall(204, 496, 592, 4)
  stageBuilder:addZone(499, 0, 2, 200, "z1")
  stageBuilder:addZone(749, 500, 2, 200, "z2")
  stageBuilder:addZone(249, 500, 2, 200, "z3")
end

ships = nil
world = nil
admin = nil
shipLaps = { }
maxLaps = 0
lapLeader = nil
bestLap = math.huge
bestLapName = nil

function init(shipsArg, worldArg, adminArg)
  ships = shipsArg
  world = worldArg
  admin = adminArg

  for i, ship in pairs(ships) do
    shipLaps[ship:name()] = { }
    shipLaps[ship:name()].laps = 0
    shipLaps[ship:name()].lastZone = nil
    shipLaps[ship:name()].lapStart = 0
    shipLaps[ship:name()].bestLap = math.huge
  end
end

function run()
  for i, ship in pairs(ships) do
    if (world:touchedZone(ship, "z2")) then
      shipLaps[ship:name()].lastZone = "z2"
    end
    if (world:touchedZone(ship, "z3")
        and shipLaps[ship:name()].lastZone == "z2") then
      shipLaps[ship:name()].lastZone = "z3"
    end
    if (world:touchedZone(ship, "z1")
        and shipLaps[ship:name()].lastZone == "z3") then
      shipLaps[ship:name()].lastZone = ""
      shipLaps[ship:name()].laps = shipLaps[ship:name()].laps + 1
      local lapTime = world:time() - shipLaps[ship:name()].lapStart
      shipLaps[ship:name()].lapStart = world:time()
      if (lapTime < shipLaps[ship:name()].bestLap) then
        shipLaps[ship:name()].bestLap = lapTime
      end
      if (lapTime < bestLap) then
        bestLap = lapTime
        bestLapName = ship:name()
      end
      if (shipLaps[ship:name()].laps > maxLaps) then
        maxLaps = shipLaps[ship:name()].laps
        lapLeader = ship:name()
      end
    end
  end
  if (maxLaps > 0) then
    admin:drawText(
        "LEADER: " .. lapLeader .. ": " .. maxLaps .. " laps", 225, 360)
    admin:drawText(
        "BEST LAP: " .. bestLapName .. ": " .. bestLap .. " ticks", 225, 310)
  end
  if (maxLaps >= LAPS) then
    for i, ship in pairs(ships) do
      admin:setScore(ship, shipLaps[ship:name()].laps)
    end
    admin:gameOver()
  end
end
