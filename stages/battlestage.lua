-- A module that defines some common functions required by battle stages.

module("battlestage", package.seeall)

local NUM_ROUNDS = 9
local ROUND_TIME_LIMIT = 5000

local currentRound = 1
local lastRoundPrinted = 0
local printRoundTimer = 0
local roundStartTime = 0

function numTeams(ships)
  local count = 0
  local teams = { }
  for i, ship in pairs(ships) do
    local teamName = ship:teamName()
    if (teams[teamName] == nil) then
      count = count + 1
      teams[teamName] = 0
    end
  end
  return count
end

function teamsAlive(ships)
  local teamsAlive = 0
  local teams = { }
  for i, ship in pairs(ships) do
    if (ship:alive()) then
      local teamName = ship:teamName()
      if (teams[teamName] == nil) then
        teamsAlive = teamsAlive + 1
        teams[teamName] = 0
      end
    end
  end
  return teamsAlive
end

local function drawRound()
  if (lastRoundPrinted < currentRound) then
    printRoundTimer = 90
    lastRoundPrinted = currentRound
  end

  if (printRoundTimer > 0) then
    admin:drawText("Round " .. currentRound, 20, 15)
    printRoundTimer = printRoundTimer - 1
  end
end

-- In free-for-all (more than 2 teams), scoring is based on kills + damage.
-- In 1v1 (or 2 teams), scoring is based on survival - the last team standing at
-- the end of each round scores a point.
-- For just 1 team, do nothing.
function basicScoring(ships, world, admin)
  drawRound()

  admin:drawText("Time: " .. world:time(), 4, -22, 16)
  local numTeams = numTeams(ships)
  if (numTeams > 2) then
    battlestage.ffaKillsPlusDamage(ships, world, admin)
  elseif (numTeams == 2) then
    battlestage.duelSurvival(ships, world, admin)
  end
end

-- When zero or one teams remain, the ship/team that has the highest score wins:
--   * 1 point per kill
--   * 1 / <default energy> points for each point of damage done
--   * negative score for damage/kills to self or team
function ffaKillsPlusDamage(ships, world, admin)
  checkRoundTimeLimit(ships, world, admin)
  local teamsAlive = teamsAlive(ships)
  if (teamsAlive <= 1) then
    if (currentRound == NUM_ROUNDS) then
      local teamScores = getTeamScores(ships, admin)
      local sortedScores = getSortedScores(teamScores)
      for i, score in ipairs(sortedScores) do
        local scoreLine = score.name .. ":  " .. round(score.total, 2) .. "  ("
            .. round(score.kills, 2) .. " destroys, " .. round(score.damage, 2)
            .. " dmg)"
        print(scoreLine)
        admin:setScore(score.name, score.total)
        admin:setStatistic(score.name, "Destroys", score.kills)
        admin:setStatistic(score.name, "Damage", score.damage)
      end
      admin:setWinner(sortedScores[1].name)
      admin:gameOver()
    else
      currentRound = currentRound + 1
      roundStartTime = world:time()
      admin:roundOver()
    end
  end
end

function getTeamScores(ships, admin)
  local numTeams = 0
  local teamScores = { }
  for i, ship in pairs(ships) do
    local teamName = ship:teamName()
    if (teamScores[teamName] == nil) then
      teamScores[teamName] = {total = 0, kills = 0, damage = 0, name = teamName}
      numTeams = numTeams + 1
    end
    local kills = admin:shipKills(ship) - admin:shipFriendlyKills(ship)
    local damage = admin:shipDamage(ship) - admin:shipFriendlyDamage(ship)
    local total = kills + damage
    teamScores[teamName].total = teamScores[teamName].total + total
    teamScores[teamName].kills = teamScores[teamName].kills + kills
    teamScores[teamName].damage = teamScores[teamName].damage + damage
  end
  return teamScores
end

local duelScores = { }

-- Each round, the last team standing scores a point. The winning team is the
-- one that won the most rounds.
function duelSurvival(ships, world, admin)
  if (not next(duelScores)) then
    for i, ship in pairs(ships) do
      duelScores[ship:teamName()] = {name = ship:teamName(), total = 0}
    end
  end

  checkRoundTimeLimit(ships, world, admin)
  local teamsAlive = teamsAlive(ships)
  if (teamsAlive <= 1) then
    local roundWinner = nil
    for i, ship in pairs(ships) do
      if (ship:alive()) then
        roundWinner = ship:teamName()
      end
    end
    if (roundWinner ~= nil) then
      duelScores[roundWinner].total = duelScores[roundWinner].total + 1
    end

    if (currentRound == NUM_ROUNDS) then
      local sortedScores = getSortedScores(duelScores)
      local teamScores = getTeamScores(ships, admin)
      for i, score in ipairs(sortedScores) do
        local scoreLine = score.name .. ": " .. score.total .. " rounds"
        print(scoreLine)
        admin:setScore(score.name, score.total)
        admin:setStatistic(score.name, "Rounds", score.total)
        admin:setStatistic(score.name, "Destroys", teamScores[score.name].kills)
        admin:setStatistic(score.name, "Damage", teamScores[score.name].damage)
      end
      admin:setWinner(sortedScores[1].name)
      admin:gameOver()
    else
      currentRound = currentRound + 1
      roundStartTime = world:time()
      admin:roundOver()
    end
  end
end

function getSortedScores(teamScores)
  local scores = { }
  for i, score in pairs(teamScores) do
    table.insert(scores, score)
  end
  table.sort(scores, scoreSorter)
  return scores
end

function scoreSorter(teamScore1, teamScore2)
  if (teamScore1.total > teamScore2.total) then
    return true
  end
  return false
end

function checkRoundTimeLimit(ships, world, admin)
  if (world:time() - roundStartTime > ROUND_TIME_LIMIT) then
    print("Round " .. currentRound .. ": time limit exceeded, destroying all "
        .. "ships")
    for i, ship in pairs(ships) do
      if (ship:alive()) then
        admin:destroyShip(ship)
      end
    end
  end
end

function round(d, x)
  local powerTen = 1
  for i = 1, x do
    powerTen = powerTen * 10
  end
  return math.floor((d * powerTen) + .5) / powerTen
end
