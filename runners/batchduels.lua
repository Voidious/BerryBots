-- Sample game runner for BerryBots. Takes as input a stage, a challenger, one
-- or more reference bots for the challenger to play against, and a number of
-- seasons. Runs <seasons> 1v1 battles of the challenger vs each reference bot
-- on the chosen stage and prints the results.

PROPERTIES_FILE = "batchduels.properties"

function run(form, runner, files, network)
  form:addStageSelect("Stage")
  form:addSingleShipSelect("Challenger")
  form:addMultiShipSelect("Reference Ships")
  form:addIntegerText("Seasons")
  form:addIntegerText("Threads")

  setDefaultReferenceShips(form, files)
  form:default("Seasons", 10) 
  form:default("Threads", 3) 
  form:default("Stage", "sample/battle1.lua")

  if (form:ok()) then
    local stage = form:get("Stage")
    local challenger = form:get("Challenger")
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
  print(teams[1] .. " vs " .. teams[2])
  if (result.winner == nil) then
    print("    Tie.")
  else
    print("    " .. result.winner .. " wins!")
  end
end
