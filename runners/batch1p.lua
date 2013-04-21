-- A Game Runner for batch single player stages, like ArcadeShooter,
-- LaserGallery, or a maze. Takes as input a stage, a challenger, and a number
-- of seasons. Runs <seasons> matches of the challenger on the chosen stage and
-- prints the results.

PROPERTIES_FILE = "batch1p.properties"

scoreKeys = { }
totalScores = { }
numWins = 0
numSeasons = nil
challenger = nil

function run(form, runner, files, network)
  form:addStageSelect("Stage")
  form:addSingleShipSelect("Challenger")
  form:addIntegerText("Seasons")
  form:addIntegerText("Threads")

  form:default("Stage", "sample/lasergallery.lua")
  form:default("Seasons", 100) 
  form:default("Threads", 2) 

  if (form:ok()) then
    local stage = form:get("Stage")
    challenger = form:get("Challenger")
    numSeasons = form:get("Seasons")
    local threadCount = form:get("Threads")
    print("Seasons: " .. numSeasons)
    print("Challenger: " .. challenger)
    print("Stage: " .. stage)
    print()
    runner:setThreadCount(threadCount)

    for i = 1, numSeasons do
      runner:queueMatch(stage, {challenger})
    end

    while (not runner:empty()) do
      processNextResult(runner)
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

function processNextResult(runner)
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
    print("    Score: " .. round(team.score, 2))
    if (team.stats ~= nil) then
      print("    Stats:")
      for key, value in pairs(team.stats) do
        print("        " .. key .. ": " .. round(value, 2))
      end
    end
    saveScore("Score", team.score)
    if (team.stats ~= nil) then
      for key, value in pairs(team.stats) do
        saveScore(key, value)
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
