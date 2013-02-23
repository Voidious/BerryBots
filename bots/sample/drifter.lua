-- A ship designed for the sample.drifter stage. Takes a very naive approach,
-- so isn't likely to get through on the first try, but usually does eventually.

ship = nil
world = nil

function init(shipRef, worldArg)
  ship = shipRef
  world = worldArg
  ship:setName("Drifter")
  ship:setShipColor(150, 150, 150)
  ship:setLaserColor(255, 255, 255)
  ship:setThrusterColor(255, 255, 255)
end

stopTimer = 30
go = true

function run(enemyShips, sensors)
  if (go and stopTimer == 0) then
    ship:fireThruster((math.pi * .25) + (math.random() * math.pi * .5), 1)
    go = false
  end
  for i, se in pairs(sensors:stageEvents()) do
    go = true
    if (se.eventType == "next") then
      -- For dramatic effect, and to make it clear to viewer than you get to
      -- make a new decision here.
      stopTimer = 30
    end
  end
  stopTimer = math.max(stopTimer - 1, 0)
end
