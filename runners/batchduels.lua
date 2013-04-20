-- A sample BerryBots Game Runner. Takes as input a stage, a challenger, one
-- or more reference bots for the challenger to play against, and a number of
-- seasons. Runs <seasons> 1v1 battles of the challenger vs each reference bot
-- on the chosen stage and prints the results.

PROPERTIES_FILE = "batchduels.properties"

scoreKeys = { }
totalScores = { }
shipKeys = { } 
scoresByShip = { }
numResults = 0
challenger = nil

function run(form, runner, files, network)
  form:addStageSelect("Stage")
  form:addSingleShipSelect("Challenger")
  form:addMultiShipSelect("Reference Ships")
  form:addIntegerText("Seasons")
  form:addIntegerText("Threads")

  setDefaultReferenceShips(form, files)
  form:default("Seasons", 10) 
  form:default("Threads", 2) 
  form:default("Stage", "sample/battle1.lua")

  if (form:ok()) then
    local stage = form:get("Stage")
    challenger = form:get("Challenger")
    local referenceShips = form:get("Reference Ships")
    local seasons = form:get("Seasons")
    local threadCount = form:get("Threads")
    print("Seasons: " .. seasons)
    print("Challenger: " .. challenger)
    print("Stage: " .. stage)
    for i, referenceShip in pairs(referenceShips) do
      print("Reference ship: " .. referenceShip)
    end
    print()
    runner:setThreadCount(threadCount)

    for i = 1, seasons do
      for j, shipName in ipairs(referenceShips) do
        runner:queueMatch(stage, {challenger, shipName})
      end
    end

    while (not runner:empty()) do
      processNextResult(runner)
    end

    print()
    print("---------------------------------------------------------------------")
    print("Overall results")
    print("---------------------------------------------------------------------")
    print("vs all ships:")
    printScores(totalScores)
    for i, ship in ipairs(shipKeys) do
      print("---------------------------------------------------------------------")
      print("vs " .. ship)
      printScores(scoresByShip[ship])
    end
  else
    -- user canceled, do nothing
  end
end

function setDefaultReferenceShips(form, files)
  if (files:exists(PROPERTIES_FILE)) then
    local configFile = files:read(PROPERTIES_FILE)
    for shipName in string.gmatch(configFile, "referenceShip=([^\n])\n") do
      form:default("Reference Ships", shipName)
    end
  else
    form:default("Reference Ships", "supersample/basicbattler.lua",
                 "sample/wallhugger.lua", "sample/randombot.lua")
  end
end

function processNextResult(runner)
  local result = runner:nextResult()
  local teams = result.teams
  print("---------------------------------------------------------------------")
  if (result.errored) then
    print("    Error: " .. result.errorMessage)
  else
    print(teams[1].name .. " vs " .. teams[2].name)
    if (result.winner == nil) then
      print("    Tie.")
    else
      print("    " .. result.winner .. " wins!")
    end
    print("------------------------------")
    local referenceShip = teams[2].name
    local sortedTeams = getSortedTeams(teams)
    for i, team in ipairs(sortedTeams) do
      if (i > 1) then
        print("--------")
      end
      print("    " .. team.name .. ":")
      print("        Rank: " .. team.rank)
      print("        Score: " .. round(team.score, 2))
      if (team.stats ~= nil) then
        print("        Stats:")
        for key, value in pairs(team.stats) do
          print("            " .. key .. ": " .. round(value, 2))
        end
      end
      if (team.name == challenger) then
        saveShipScore(referenceShip, "Rank", team.rank)
        saveShipScore(referenceShip, "Score", team.score)
        if (team.stats ~= nil) then
          for key, value in pairs(team.stats) do
            saveShipScore(referenceShip, key, value)
          end
        end
        numResults = numResults + 1
      end
    end
  end
end

function printScores(scores)
  for i, key in ipairs(scoreKeys) do
    print("    " .. key .. ": " .. round(scores[key] / numResults, 2))
  end
end

function saveShipScore(ship, key, value)
  if (scoresByShip[ship] == nil) then
    scoresByShip[ship] = { }
  end
  saveShipKey(ship)
  saveScoreKey(key)
  saveScore(scoresByShip[ship], key, value)
  saveTotalScore(key, value)
end

function saveTotalScore(key, value)
  saveScore(totalScores, key, value)
end

function saveScore(scores, key, value)
  if (scores[key] == nil) then
    scores[key] = value
  else
    scores[key] = scores[key] + value
  end
end

function saveScoreKey(newKey)
  saveKey(scoreKeys, newKey)
end

function saveShipKey(newKey)
  saveKey(shipKeys, newKey)
end

function saveKey(keys, newKey)
  for i, key in pairs(keys) do
    if (key == newKey) then
      return
    end
  end
  table.insert(keys, newKey)
end

function getSortedTeams(teams)
  local sortedTeams = { }
  for i, team in pairs(teams) do
    table.insert(sortedTeams, team)
  end
  table.sort(sortedTeams, teamSorter)
  return sortedTeams
end

function teamSorter(team1, team2)
  if (team1.rank < team2.rank or (team1.rank > 0 and team2.rank == 0)) then
    return true
  end
  return false
end

function round(d, x)
  local powerTen = 1
  for i = 1, x do
    powerTen = powerTen * 10
  end
  return math.floor((d * powerTen) + .5) / powerTen
end
