-- A module that defines some common functions required by BerryBots battle
-- stages.

module(..., package.seeall);

-- When zero or one bots remain, the bot that has the highest score wins:
--   * 1 point per kill
--   * 1 / <default energy> points for each point of damage done
--   * negative score for damage/kills to self or team

local currentRound = 1
local numRounds = 9
local teamScores = { }

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

function basicScoring(ships, admin)
  local numTeams = numTeams(ships)

  if (numTeams > 2) then
    battlestage.ffaKillsPlusDamage(ships, admin)
  else
    battlestage.duelSurvival(ships, admin)
  end
end

function ffaKillsPlusDamage(ships, admin)
  local teamsAlive = teamsAlive(ships)
  if (teamsAlive <= 1) then
    if (currentRound == numRounds) then
      local numTeams = 0
      local teamScores = { }
      for i, ship in pairs(ships) do
        local teamName = ship:teamName()
        if (teamScores[teamName] == nil) then
          teamScores[teamName] = 0
          numTeams = numTeams + 1
        end
        local kills = admin:shipKills(ship) - admin:shipFriendlyKills(ship)
        local damage = admin:shipDamage(ship) - admin:shipFriendlyDamage(ship)
        local score = kills + damage
        print(ship:name() .. ": " .. round(score, 2) .. " (" .. round(kills, 2)
            .. " kills/" .. round(damage, 2) .. " damage)")
        teamScores[teamName] = teamScores[teamName] + score
      end

      if (numTeams < (# ships)) then
        print("****")
        for teamName, teamScore in pairs(teamScores) do
          print(teamName .. ": " .. round(teamScore, 2))
        end
      end
      local bestScore = -math.huge
      local winner = nil
      for teamName, teamScore in pairs(teamScores) do
        if (teamScore > bestScore) then
          bestScore = teamScore
          winner = teamName
        end
      end
      admin:setWinner(winner)
      admin:gameOver()
    else
      currentRound = currentRound + 1
      admin:roundOver()
    end
  end
end

function duelSurvival(ships, admin)
  if (not next(teamScores)) then
    for i, ship in pairs(ships) do
      teamScores[ship:teamName()] = 0
    end
  end
  
  local teamsAlive = teamsAlive(ships)
  if (teamsAlive <= 1) then
    local roundWinner = nil
    for i, ship in pairs(ships) do
      if (ship:alive()) then
        roundWinner = ship:teamName()
      end
    end
    if (roundWinner ~= nil) then
      teamScores[roundWinner] = teamScores[roundWinner] + 1
    end

    if (currentRound == numRounds) then
      local team1 = nil
      local team2 = nil
      local score1
      local score2
      for teamName, teamScore in pairs(teamScores) do
        print(teamName .. ": " .. teamScore)
        if (team1 == nil) then
          team1 = teamName
          score1 = teamScore
        else
          team2 = teamName
          score2 = teamScore
        end
      end
      if (score1 > score2) then
        admin:setWinner(team1)
      elseif (score2 > score1) then
        admin:setWinner(team2)
      end
      admin:gameOver()
    else
      currentRound = currentRound + 1
      admin:roundOver()
    end
  end
end

function round(d, x)
  local powerTen = 1;
  for i = 1, x do
    powerTen = powerTen * 10
  end
  return math.floor((d * powerTen) + .5) / powerTen
end
