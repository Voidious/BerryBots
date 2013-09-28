-- A Game Runner for batch single player stages, like ArcadeShooter,
-- LaserGallery, or a maze. Takes as input a stage, a challenger, and a number
-- of seasons. Runs <seasons> matches of the challenger on the chosen stage and
-- prints the results.

SETTINGS_FILE = "settings/batch1p.properties"

scoreKeys = { }
totalScores = { }
numWins = 0
numSeasons = nil
challenger = nil

function run(runner, form, files, network)
  form:addStageSelect("Stage")
  form:addSingleShipSelect("Challenger")
  form:addIntegerText("Seasons")
  form:addIntegerText("Threads")
  form:addCheckbox("Save replays")
  defaultSettings(form, files)

  if (form:ok()) then
    local stage = form:get("Stage")
    challenger = form:get("Challenger")
    numSeasons = form:get("Seasons")
    local threadCount = form:get("Threads")
    local saveReplays = form:get("Save replays")
    saveSettings(files, stage, challenger, numSeasons, threadCount, saveReplays)

    print("Seasons: " .. numSeasons)
    print("Challenger: " .. challenger)
    print("Stage: " .. stage)
    print()

    runner:setThreadCount(threadCount)
    for i = 1, numSeasons do
      runner:queueMatch(stage, {challenger})
    end

    while (not runner:empty()) do
      processNextResult(runner, saveReplays)
    end

    print()
    print("---------------------------------------------------------------------")
    print("Overall results")
    print("---------------------------------------------------------------------")
    printScores()
  else
    -- user canceled, do nothing
  end
end

function defaultSettings(form, files)
  if (files:exists(SETTINGS_FILE)) then
    local settingsFile = files:read(SETTINGS_FILE)
    local stage, challenger, seasons, threads, t
    t, t, stage = string.find(settingsFile, "Stage=([^\n]*)\n")
    t, t, challenger = string.find(settingsFile, "Challenger=([^\n]*)\n")
    t, t, seasons = string.find(settingsFile, "Seasons=([^\n]*)\n")
    t, t, threads = string.find(settingsFile, "Threads=([^\n]*)\n")
    t, t, saveReplays = string.find(settingsFile, "Replays=([^\n]*)\n")
    saveReplays = (saveReplays == "true")

    if (stage ~= nil) then form:default("Stage", stage) end
    if (challenger ~= nil) then form:default("Challenger", challenger) end
    if (seasons ~= nil) then form:default("Seasons", seasons) end
    if (threads ~= nil) then form:default("Threads", threads) end
    if (saveReplays ~= nil) then form:default("Save replays", saveReplays) end
  else
    form:default("Stage", "sample/lasergallery.lua")
    form:default("Seasons", 100) 
    form:default("Threads", 2) 
  end
end

function saveSettings(files, stage, challenger, numSeasons, threadCount,
                      saveReplays)
  local settings = "Stage=" .. stage .. "\n"
      .. "Challenger=" .. challenger .. "\n"
      .. "Seasons=" .. numSeasons .. "\n"
      .. "Threads=" .. threadCount .. "\n"
      .. "Replays=" .. tostring(saveReplays) .. "\n"
  files:write(SETTINGS_FILE, settings)
end

function processNextResult(runner, saveReplay)
  local result = runner:nextResult()
  print("---------------------------------------------------------------------")
  if (result.errored) then
    print("    Error: " .. result.errorMessage)
  else
    if (result.winner == nil) then
      print("    Loss.")
    elseif (result.winner == challenger) then
      print("    Win.")
      numWins = numWins + 1
    end
    local team = result.teams[1]
    if (team.score ~= nil) then
      print("    Score: " .. round(team.score, 2))
      saveScore("Score", team.score)
    end
    if (team.stats ~= nil) then
      print("    Stats:")
      for key, value in pairs(team.stats) do
        print("        " .. key .. ": " .. round(value, 2))
        saveScore(key, value)
      end
    end
    if (saveReplay) then
      local replayName = runner:saveReplay()
      if (replayName ~= nil) then
        print("    Replay saved to: " .. replayName)
      end
    end
  end
end

function printScores()
  print("    Wins: " .. numWins .. " / " .. numSeasons)
  for i, keyEntry in ipairs(scoreKeys) do
    print("    " .. keyEntry.key .. ": "
          .. round(totalScores[keyEntry.key] / keyEntry.count, 2))
  end
end

function saveScore(key, value)
  local foundKey = false
  for i, keyEntry in pairs(scoreKeys) do
    if (keyEntry.key == key) then
      keyEntry.count = keyEntry.count + 1
      foundKey = true
    end
  end
  if (not foundKey) then
    table.insert(scoreKeys, {key = key, count = 1})
  end

  if (totalScores[key] == nil) then
    totalScores[key] = value
  else
    totalScores[key] = totalScores[key] + value
  end

end

function round(d, x)
  local powerTen = 1
  for i = 1, x do
    powerTen = powerTen * 10
  end
  return math.floor((d * powerTen) + .5) / powerTen
end
