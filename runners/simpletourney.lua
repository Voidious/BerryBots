-- A Game Runner for a bracket tournament of duels (or 2-team matches). Takes as
-- input a stage, 2 or more ship control programs, and number of matches per
-- pairing. Runs a single-elimination tournament bracket with random seeds and
-- prints the results.

SETTINGS_FILE = "settings/simpletourney.properties"

shipData = { }

function run(runner, form, files, network)
  form:addStageSelect("Stage")
  form:addMultiShipSelect("Ships")
  form:addIntegerText("Best of...")
  form:addDropdown("Seeds", {"Random", "Round-robin"})
  form:addIntegerText("Threads")
  form:addCheckbox("Save replays")
  defaultSettings(form, files)

  local stage = nil
  local ships = nil
  local matches = nil
  local seeds = nil
  local threadCount = nil
  local saveReplays = false

  local ok = false
  local done = false
  local message = ""
  while (not done) do
    if (form:ok(message)) then
      ok = true
      stage = form:get("Stage")
      ships = form:get("Ships")
      matches = form:get("Best of...")
      seeds = form:get("Seeds")
      threadCount = form:get("Threads")
      saveReplays = form:get("Save replays")
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

  saveSettings(files, stage, ships, matches, seeds, threadCount, saveReplays)

  print("Stage: " .. stage)
  for i, ship in pairs(ships) do
    print("Ship: " .. ship)
  end
  print("Best of: " .. matches)
  print("Seeds: " .. seeds)

  runner:setThreadCount(threadCount)

  processSeeds(runner, stage, ships, matches, seeds, threadCount, saveReplays)

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

    if (saveReplays) then print() end

    -- Process individual match results
    local numShips = math.min(slots, (# shipData))
    for y = 1, numShips do
      shipData[y].wins = 0
    end
    while (not runner:empty()) do
      local result = runner:nextResult()
      if (saveReplays) then
        printSavedReplay(result, runner:saveReplay())
      end
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
          if (saveReplays) then
            printSavedReplay(result, runner:saveReplay())
          end
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

function defaultSettings(form, files)
  if (files:exists(SETTINGS_FILE)) then
    local settingsFile = files:read(SETTINGS_FILE)
    local stage, matches, threads, t
    t, t, stage = string.find(settingsFile, "Stage=([^\n]*)\n")
    t, t, matches = string.find(settingsFile, "Matches=([^\n]*)\n")
    t, t, seeds = string.find(settingsFile, "Seeds=([^\n]*)\n")
    t, t, threads = string.find(settingsFile, "Threads=([^\n]*)\n")
    t, t, saveReplays = string.find(settingsFile, "Replays=([^\n]*)\n")
    saveReplays = (saveReplays == "true")

    if (stage ~= nil) then form:default("Stage", stage) end
    if (matches ~= nil) then form:default("Best of...", matches) end
    if (seeds ~= nil) then form:default("Seeds", seeds) end
    if (threads ~= nil) then form:default("Threads", threads) end
    if (saveReplays ~= nil) then form:default("Save replays", saveReplays) end
    for ship in string.gmatch(settingsFile, "Ship=([^\n]*)\n") do
      form:default("Ships", ship)
    end
  else
    form:default("Stage", "sample/battle1.lua")
    form:default("Best of...", 7) 
    form:default("Threads", 2) 
  end
end

function saveSettings(files, stage, ships, matches, seeds, threadCount,
                      saveReplays)
  local settings = "Stage=" .. stage .. "\n"
  for i, ship in ipairs(ships) do
    settings = settings .. "Ship=" .. ship .. "\n"
  end
  settings = settings .. "Matches=" .. matches .. "\n"
      .. "Threads=" .. threadCount .. "\n"
      .. "Replays=" .. tostring(saveReplays) .. "\n"
      .. "Seeds=" .. seeds .. "\n"
  files:write(SETTINGS_FILE, settings)
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
  return (ship1.seedValue > ship2.seedValue)
end

function printSavedReplay(result, replayName)
  local teamString = nil
  for i, team in ipairs(result.teams) do
    if (teamString == nil) then
      teamString = team.name
    else
      teamString = teamString .. " vs " .. team.name
    end      
  end
  print("    " .. teamString)
  print("    " .. result.winner .. " wins!")
  print("    Replay saved to: " .. replayName)
  print()
end

function processSeeds(runner, stage, ships, matches, seeds, threadCount,
    saveReplays)
  if (seeds == "Random") then
    for i, ship in pairs(ships) do
      table.insert(shipData, {name=ship, seedValue=math.random(), seed=0})
    end
  elseif (seeds == "Round-robin") then
    print()
    print("Processing round-robin...")
    for i, ship1 in ipairs(ships) do
      for j, ship2 in ipairs(ships) do
        if (i < j) then
          for k = 1, matches do
            runner:queueMatch(stage, {ship1, ship2})
          end
        end
      end
    end

    local shipWins = {}   
    while (not runner:empty()) do
      local result = runner:nextResult()
      local printed = false
      if (saveReplays) then
        printSavedReplay(result, runner:saveReplay())
        printed = true
      end

      local winner = result.winner

      if (winner ~= nil) then
        local loser = nil
        for i, team in pairs(result.teams) do
          if (team.name ~= winner) then
            loser = team.name
          end
        end
        if (not printed) then
          print("  " .. winner .. " defeats " .. loser)
        end
        local entry = shipWins[winner]
        if (entry == nil) then
          entry = {wins = 0}
          shipWins[winner] = entry
        end
        entry.wins = entry.wins + 1
      end
    end

    print()
    print("Round-robin results:")
    for i, ship in pairs(ships) do
      local entry = shipWins[ship]
      local seedValue = 0
      if (entry ~= nil) then
        seedValue = entry.wins
      end
      table.insert(shipData, {name=ship, seedValue=seedValue, seed=0})
      print("  " .. ship .. ": " .. seedValue .. " wins")
    end 
  end

  shipData = getSortedShips(shipData)
  for i, shipDatum in ipairs(shipData) do
    shipDatum.seed = i
  end
end
