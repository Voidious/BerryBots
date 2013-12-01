-- A Game Runner that uses a genetic algorithm to evolve a ship that can solve a
-- maze.
--
-- The maze is a version of sample.maze2 modified to give additional
-- metrics as results: percentage of the stage visited, % seen (i.e., by line
-- of sight), and % zones seen. Any maze could be outfitted with the same
-- metrics. But to solve many mazes, you would probably need to improve upon
-- this program. Fitness is calculated by the fitnessValue() function.
--
-- The "DNA" string is a series of bits that represents a sequence of
-- movements. 2 bits = 4 options = up / down / left / right. Each movement
-- has ACCEL_TIME ticks of acceleration, specified in mazerbase.lua.txt. This is
-- a fairly silly way to solve a maze and not a terribly efficient genetic
-- algorithm.
--
-- Other metrics could help, like penalizing back-tracking or total
-- displacement. Even better would be a system where the bit string is inputs
-- into an algorithm that dictates behavior - e.g., get this close to a wall,
-- follow it for this long, always follow an opening to the south if you see
-- one, etc. That might be more capable of improvement through crossover.
-- With the bit string as a movement sequence, most improvement probably happens
-- via mutation.

POPULATION_SIZE = 20
MUTATION_RATE = 0.001
CROSSOVER_RATE = 0.95
MAX_GENERATIONS = 10000

CHUNK_SIZE = 2  -- 2 bits to represent up, down, left, right
BITS = CHUNK_SIZE * 512
MOVES = {}
MOVES["00"] = "0"
MOVES["01"] = "(math.pi / 2)"
MOVES["10"] = "math.pi"
MOVES["11"] = "(math.pi * 3 / 2)"

function run(runner, form, files)
  runner:setThreadCount(3)
  files:writeStage("maze2e.lua", files:read("mazer/maze2e.lua.txt"))

  local members = createPopulation(POPULATION_SIZE)
  for i = 1, MAX_GENERATIONS do
    members = nextGeneration(members, runner, files)
  end
end

function createPopulation(size)
  local members = {}
  for i = 1, size do
    table.insert(members, randomBitString())
  end
  return members
end

function nextGeneration(members, runner, files)
  local memberDataMap = {}
  for i = 1, (# members) do
    local filename = "mazer" .. i .. ".lua"
    memberDataMap["runners/" .. filename] = {bits = members[i]}
    files:writeBot(filename, mazerCode(members[i], files))
    runner:queueMatch("runners/maze2e.lua", {"runners/" .. filename})
  end

  while (not runner:empty()) do
    local result = runner:nextResult()
    local name = result.teams[1].name
    local stats = result.teams[1].stats
    memberDataMap[name].name = name
    memberDataMap[name].fitnessValue = fitnessValue(stats["Grid Visible"],
        stats["Zone Visible"], stats["Grid Visited"], (result.winner ~= nil))
    print("Member data: " .. memberDataMap[name].fitnessValue
          .. string.len(memberDataMap[name].bits))
  end

  local memberData = {}
  for i, memberDatum in pairs(memberDataMap) do
    table.insert(memberData, memberDatum)
  end
  sortMemberData(memberData)
  print("--------")
  print("  members = {")
  for i, memberDatum in ipairs(memberData) do
    print("    \"" .. memberDatum.bits .. "\",")
  end
  print("  }")
  print("--------")
  print("Best fitness: " .. memberData[1].fitnessValue)

  print()
  return breedAndMutate(memberData)
end

function fitnessValue(gridVisible, zoneVisible, gridVisited, win)
  local value = gridVisible + gridVisited + (1000 * zoneVisible)
  if (win) then
    value = value + 1000000
  end
  return value
end

function sortMemberData(memberData)
  table.sort(memberData, memberDataSorter)
end

function memberDataSorter(memberDatum1, memberDatum2)
  return memberDatum1.fitnessValue > memberDatum2.fitnessValue
end

function breedAndMutate(memberData)
  setRouletteRanges(memberData)
  local len = (# memberData)
  local members = {}
  while ((# members) < len) do
    local member1 = rouletteMember(memberData, math.random())
    local member2 = nil
    local tries = 0
    repeat
      member2 = rouletteMember(memberData, math.random())
      tries = tries + 1
    until (member1 ~= member2 or tries > 1000)
    if (member1 == member2) then
      member2 = memberData[math.floor(math.random() * len) + 1]
    end

    if (math.random() < CROSSOVER_RATE) then
      local numNewMembers = math.min(2, len - (# members))
      for i = 1, numNewMembers do
        local newMember = mutate(crossover(member1, member2))
        table.insert(members, newMember)
      end
    else
      table.insert(members, member1.bits)
      table.insert(members, member2.bits)
    end
  end

  return members
end

function crossover(memberDatum1, memberDatum2)
  local crossoverPoint = math.floor(math.random() * (string.len(memberDatum1.bits) - 1)) + 1
  local members = {memberDatum1.bits, memberDatum2.bits}
  local x = 1
  if (math.random() < 0.5) then
    x = 2
  end

  local s = string.sub(members[x], 1, crossoverPoint)
  x = ((x - 1) * -1) + 2
  return s .. string.sub(members[x], crossoverPoint + 1, (# members[x]))
end

function mutate(member)
  local len = string.len(member)
  for i = 1, len do
    if (math.random() < MUTATION_RATE) then
      local bitSections = {}
      if (i > 1) then
        table.insert(bitSections, string.sub(member, 1, i - 1))
      end

      local bit = string.sub(member, i, i)
      if (bit == "0") then
        table.insert(bitSections, "1")
      else
        table.insert(bitSections, "0")
      end

      if (i < len) then
        table.insert(bitSections, string.sub(member, i + 1, len))
      end
      member = table.concat(bitSections, "")
    end
  end
  return member
end

function setRouletteRanges(memberData)
  local min = math.huge
  for i, memberDatum in ipairs(memberData) do
    min = math.min(min, memberDatum.fitnessValue)
  end

  local sum = 0
  for i, memberDatum in ipairs(memberData) do
    sum = sum + math.max(0, memberDatum.fitnessValue - min)
  end    

  local rangeSum = 0
  for i, memberDatum in ipairs(memberData) do
    memberDatum.rouletteRange =
        rangeSum + (math.max(0, memberDatum.fitnessValue - min) / sum)
    rangeSum = rangeSum + memberDatum.rouletteRange
  end
end

function rouletteMember(memberData, rouletteValue)
  local numMembers = (# memberData)
  for i = 1, numMembers do
    if (i == numMembers or ((rouletteValue >= memberData[i].rouletteRange)
            and (rouletteValue < memberData[i + 1].rouletteRange))) then
      return memberData[i]
    end
  end
  return nil
end

function mazerCode(bitString, files)
  local m = {}
  local len = (# bitString) / CHUNK_SIZE
  for i = 1, len do
    local moveBits =
        string.sub(bitString, (i * CHUNK_SIZE) - 1, (i * CHUNK_SIZE))
    table.insert(m, MOVES[moveBits])
  end

  return files:read("mazer/mazerbase.lua.txt") .. "\nmoves = {"
      .. table.concat(m, ", ") .. "}\n"
end

function randomBitString()
  local s = {}
  for i = 1, BITS do
    if (math.random() < 0.5) then
      table.insert(s, "0")
    else
      table.insert(s, "1")
    end
  end
  return table.concat(s, "")
end
