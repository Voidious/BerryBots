-- A Game Runner for a bracket tournament of duels (or 2-team matches). Takes as
-- input a stage, 2 or more ship control programs, and number of matches per
-- pairing. Runs a single-elimination tournament bracket with random seeds and
-- prints the results.

shipData = { }

function run(form, runner, files, network)
  form:addStageSelect("Stage")
  form:addMultiShipSelect("Ships")
  form:addIntegerText("Best of...")
  form:addIntegerText("Threads")

  form:default("Stage", "sample/battle1.lua")
  form:default("Best of...", 7) 
  form:default("Threads", 2) 

  local stage = nil
  local ships = nil
  local matches = nil
  local threadCount = nil

  local ok = false
  local done = false
  local message = ""
  while (not done) do
    if (form:ok(message)) then
      ok = true
      stage = form:get("Stage")
      ships = form:get("Ships")
      matches = form:get("Best of...")
      threadCount = form:get("Threads")
      if (# ships >= 2) then
        done = true
      else
        message = "At least 2 ship programs required for a tournament!"
      end
    else
      done = true
    end
  end

  if (not ok) then
    return
  end

  -- TODO: Add option for pool play to determine seeds instead of random seeds.

  print("Stage: " .. stage)
  for i, ship in pairs(ships) do
    print("Ship: " .. ship)
  end
  print("Best of: " .. matches)

  runner:setThreadCount(threadCount)
  for i, ship in pairs(ships) do
    table.insert(shipData, {name=ship, seedValue = math.random(), seed = 0})
  end
  shipData = getSortedShips(shipData)
  for i, shipDatum in ipairs(shipData) do
    shipDatum.seed = i
  end

  local rounds = 1
  local slots = 2
  while (slots < (# shipData)) do
    rounds = rounds + 1
    slots = slots * 2
  end

  for x = 1, rounds do
    print()
    print("-------------------------------------------------------------------")
    if (x < rounds) then
      print("Round " .. x)
    else
      print("Finals")
    end
    print("-------------------------------------------------------------------")

    -- Print matchups and queue matches
    for y = 1, (slots / 2) do
      print()
      local effectiveSeed1 = y
      local effectiveSeed2 = slots - y + 1
      if ((# shipData) < effectiveSeed2) then
        print("Seed " .. effectiveSeed1 .. " (" .. shipData[effectiveSeed1].name
              .. ") gets a bye")
      else
        local higherSeed = shipData[effectiveSeed1]
        local lowerSeed = shipData[effectiveSeed2]
      
        print(higherSeed.seed .. "  " .. higherSeed.name)
        print(lowerSeed.seed .. "  " .. lowerSeed.name)

        for z = 1, matches do
          runner:queueMatch(stage, {higherSeed.name, lowerSeed.name})
        end
      end
    end

    -- Process individual match results
    local numShips = math.min(slots, (# shipData))
    for y = 1, numShips do
      shipData[y].wins = 0
    end
    while (not runner:empty()) do
      local result = runner:nextResult()
      for y = 1, numShips do
        local shipDatum = shipData[y]
        if (result.winner == shipDatum.name) then
          shipDatum.wins = shipDatum.wins + 1
        end
      end
    end

    -- Process matchup scores, run tie-breakers, and print results
    print()
    for y = 1, (slots / 2) do
      local effectiveSeed1 = y
      local effectiveSeed2 = slots - y + 1
      if ((# shipData) >= effectiveSeed2) then
        local higherSeed = shipData[effectiveSeed1]
        local lowerSeed = shipData[effectiveSeed2]

        while (higherSeed.wins == lowerSeed.wins) do
          print("    Queueing tie-breaker for " .. higherSeed.name .. " vs "
                .. lowerSeed.name .. "...")
          runner:queueMatch(stage, {higherSeed.name, lowerSeed.name})
          local result = runner:nextResult()
          if (result.winner == higherSeed.name) then
            higherSeed.wins = higherSeed.wins + 1
          elseif (result.winner == lowerSeed.name) then
            lowerSeed.wins = lowerSeed.wins + 1
          end
        end

        local winner, loser
        if (lowerSeed.wins > higherSeed.wins) then
          shipData[effectiveSeed1] = lowerSeed
          winner, loser = lowerSeed, higherSeed
        else
          winner, loser = higherSeed, lowerSeed
        end
        print(winner.name .. " defeats " .. loser.name .. ": "
              .. winner.wins .. " - " .. loser.wins)
      end
    end

    slots = slots / 2
  end
end

function getSortedShips(ships)
  local sortedShips = { }
  for i, team in pairs(ships) do
    table.insert(sortedShips, team)
  end
  table.sort(sortedShips, shipSorter)
  return sortedShips
end

function shipSorter(ship1, ship2)
  if (ship1.seedValue < ship2.seedValue
      or (ship1.seedValue > 0 and ship2.seedValue == 0)) then
    return true
  end
  return false
end
