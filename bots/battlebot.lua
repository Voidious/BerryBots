-- A module that defines some common functions required by battle ships.

module("battlebot", package.seeall)

function newLine(x1, y1, x2, y2)
  local wallLine = { }
  if (x1 == x2) then
    wallLine.m = math.huge
    wallLine.b = math.huge
    wallLine.xMin, wallLine.xMax = x1, x1
  else
    wallLine.m = (y2 - y1) / (x2 - x1)
    wallLine.b = y1 - (wallLine.m * x1)
    wallLine.xMin = math.min(x1, x2)
    wallLine.xMax = math.max(x1, x2)
  end
  wallLine.yMin = math.min(y1, y2)
  wallLine.yMax = math.max(y1, y2)
  wallLine.x1, wallLine.y1, wallLine.x2, wallLine.y2 = x1, y1, x2, y2

  return wallLine
end

function initWalls(walls)
  local wallLines = { }
  for i, wall in pairs(walls) do
    local x1, y1 = wall.left, wall.bottom
    local x2, y2 = wall.left + wall.width, wall.bottom
    local x3, y3 = wall.left + wall.width, wall.bottom + wall.height
    local x4, y4 = wall.left, wall.bottom + wall.height
    table.insert(wallLines, newLine(x1, y1, x2, y2))
    table.insert(wallLines, newLine(x2, y2, x3, y3))
    table.insert(wallLines, newLine(x3, y3, x4, y4))
    table.insert(wallLines, newLine(x4, y4, x1, y1))
  end
  return wallLines
end

function isVisible(wallLines, srcShip, x, y)
  local visionLine = newLine(x, y, srcShip:x(), srcShip:y())
  for i, wallLine in pairs(wallLines) do
    if (intersects(wallLine, visionLine)) then
      return false
    end
  end
  return true
end

function intersects(line1, line2, buffer)
  if (buffer == nil) then
    buffer = 0
  end
  local m1 = line1.m
  local m2 = line2.m
  if (m1 == m2) then
    return false
  elseif (m1 == math.huge or m2 == math.huge) then
    if (m1 == math.huge and m2 == 0) then
      return (line1.xMin >= line2.xMin - buffer
          and line1.xMax <= line2.xMax + buffer
          and line1.yMin <= line2.yMin + buffer
          and line1.yMax >= line2.yMax - buffer)
    else
      return intersects(inverse(line1), inverse(line2), buffer)
    end
  end
  
  local x = (line2.b - line1.b) / (line1.m - line2.m)
  local y = line1.m * x + line1.b
  return (x >= line1.xMin - buffer and x <= line1.xMax + buffer
      and x >= line2.xMin - buffer and x <= line2.xMax + buffer
      and y >= line1.yMin - buffer and y <= line1.yMax + buffer
      and y >= line2.yMin - buffer and y <= line2.yMax + buffer)
end

function inverse(line)
  return newLine(line.y1, line.x1, line.y2, line.x2)
end

function sign(x)
  if (x > 0) then
    return 1
  elseif (x < 0) then
    return -1
  else
    return 0
  end
end

function distance(x1, y1, x2, y2)
  return math.sqrt(distanceSq(x1, y1, x2, y2))
end

function distanceSq(x1, y1, x2, y2)
  return square(x1 - x2) + square(y1 - y2)
end

function square(x)
  return x * x
end

function normalRelativeAngle(x)
  while (x >= math.pi) do
    x = x - (2 * math.pi)
  end
  while (x < -math.pi) do
    x = x + (2 * math.pi)
  end
  return x
end

function normalAbsoluteAngle(x)
  while (x >= 2 * math.pi) do
    x = x - (2 * math.pi)
  end
  while (x < 0) do
    x = x + (2 * math.pi)
  end
  return x
end

function absoluteBearing(x1, y1, x2, y2)
  return math.atan2(y2 - y1, x2 - x1)
end
