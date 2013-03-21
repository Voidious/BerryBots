-- A module that defines some common functions required by the sample stages.

module("samplestage", package.seeall)

function checkSinglePlayer(ships, admin)
  checkNumPlayers(1, ships, admin)
end

function checkNumPlayers(expectedPlayers, ships, admin)
  local allTeams = { }
  local playerTeams = { }
  local numPlayers = 0
  for i, ship in ipairs(ships) do
    if (not ship:isStageShip() and playerTeams[ship:teamName()] == nil) then
      if (numPlayers < expectedPlayers) then
        allTeams[ship:teamName()] = true
        playerTeams[ship:teamName()] = true
      else
        allTeams[ship:teamName()] = true
        if (ship:alive()) then
          admin:destroyShip(ship)
          print("Destroying extra player ship: " .. ship:name())
        end
      end
      numPlayers = numPlayers + 1
    end
  end

  if (numPlayers ~= expectedPlayers) then
    local warning = "*** This stage is designed for " .. expectedPlayers .. " team"
    if (expectedPlayers > 1) then warning = warning .. "s" end
    if (numPlayers > expectedPlayers) then
      warning = warning .. ", destroying the rest"
    end
    admin:drawText(warning .. " ***", 4, -25, 14)
  end
end
