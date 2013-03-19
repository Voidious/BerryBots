-- A sample ship that can move through most mazes fairly efficiently, albeit
-- somewhat awkwardly.
--
-- This is the only sample ship with any real pathfinding for navigating walls.

ship = nil
gfx = nil
zones = nil
width = nil
height = nil
shipRadius = 18 -- 8 + some buffer

gridSize = 50
gridPoints = { }
wallLines = { }

targetx, targety, targetPoint = nil, nil, nil

function init(shipArg, world, gfxArg)
  ship = shipArg
  walls = world:walls()
  zones = world:zones()
  width = world:width()
  height = world:height()
  gfx = gfxArg
  ship:setName("Snail")
  ship:setShipColor(0, 0, 255)
  ship:setLaserColor(0, 0, 0)
  ship:setThrusterColor(0, 0, 255)

  if (# zones == 0) then
    print("No target set, nothing to do.")
  else
    i, zone = next(zones)
    targetx = zone.left + (zone.width / 2)
    targety = zone.bottom + (zone.height / 2)
    print("Target: " .. targetx .. ", " .. targety)
    buildPathGraph()
  end
  drawGrid(nearestGridPoint(ship:x(), ship:y()))
end

function buildPathGraph()
  print("Building grid...")
  for i = 1, (width / gridSize - 1) do
    for j = 1, (height / gridSize - 1) do
      table.insert(gridPoints, Point.create(i * gridSize, j * gridSize))
    end
  end
  targetPoint = Point.create(targetx, targety)
  table.insert(gridPoints, targetPoint)
  print(# gridPoints .. " grid points created.")
  print()

  print("Constructing lines for walls...")
  for i, wall in pairs(walls) do
    table.insert(wallLines, Line.create(
        wall.left - shipRadius,
        wall.bottom - shipRadius,
        wall.left + wall.width + shipRadius,
        wall.bottom - shipRadius))
    table.insert(wallLines, Line.create(
        wall.left + wall.width + shipRadius,
        wall.bottom - shipRadius,
        wall.left + wall.width + shipRadius,
        wall.bottom + wall.height + shipRadius))
    table.insert(wallLines, Line.create(
        wall.left + wall.width + shipRadius,
        wall.bottom + wall.height + shipRadius,
        wall.left - shipRadius,
        wall.bottom + wall.height + shipRadius))
    table.insert(wallLines, Line.create(
        wall.left - shipRadius,
        wall.bottom + wall.height + shipRadius,
        wall.left - shipRadius,
        wall.bottom - shipRadius))
  end
  print(# wallLines .. " wall lines created.")
  print()

  print("Calculating paths...")
  for i,p1 in pairs(gridPoints) do
    for j,p2 in pairs(gridPoints) do
      if (p1 ~= p2 and p1:isAdjacentTo(p2) and p1:pathExistsTo(p2)) then
        p1:addEdge(p2)
        p2:addEdge(p1)
      end
    end
  end
  print("Done.")
  print()
end

function validPath(p1x, p1y, p2x, p2y)
  pathLine = Line.create(p1x, p1y, p2x, p2y)
  for i,wallLine in pairs(wallLines) do
    if (pathLine:intersects(wallLine)) then
      return false
    end
  end
  return true
end

destination = nil

function run()
  if (targetx ~= nil) then
    if (destination == nil or distanceSqToPoint(destination) < 10) then
      source = nearestGridPoint(ship:x(), ship:y())
      if (source ~= nil) then
        destination = nextPointOnPath(source)
        if (destination == nil) then
          destination = source
        end
      end
    end
    if (destination ~= nil and ship:speed() > 1
        and math.abs(normalRelativeAngle(
            bearingToPoint(destination) - ship:heading())) > 0.1) then
      ship:fireThruster(ship:heading() + math.pi, ship:speed())
    else
      thrust = 5 - ship:speed()
      ship:fireThruster(bearingToPoint(destination), thrust)
    end
    if (gfx:enabled()) then
      drawPath(source, destination)
    end
  end
end

function nearestGridPoint(x, y)
  nearestDistanceSq = math.huge
  nearest = nil
  for i, p in pairs(gridPoints) do
    distSq = square(p.x - x) + square(p.y - y)
    if (p ~= targetPoint and distSq < nearestDistanceSq
        and validPath(x, y, p.x, p.y)) then
      nearestDistanceSq = distSq
      nearest = p
    end
  end
  return nearest
end

function nextPointOnPath(p)
  source = nil
  for i, gridPoint in pairs(gridPoints) do
    if (p:distanceSq(gridPoint) < 25) then
      source = gridPoint
    else
      gridPoint.distance = math.huge
      gridPoint.previous = nil
      gridPoint.visited = false
    end
  end
  if (source == nil) then
    return nil
  end
  
  done = false
  source.distance = 0
  source.visited = false
  thisPoint = source
  while (not done) do
    for i, edgePoint in pairs(thisPoint.edges) do
      if (thisPoint.distance + 1 < edgePoint.distance) then
        edgePoint.distance = thisPoint.distance + 1
        edgePoint.previous = thisPoint
      end
      if (edgePoint == targetPoint) then
        done = true
      end
    end
    thisPoint.visited = true
    thisPoint = nextUnvisitedGridPoint()
    if (thisPoint == nil) then
      done = true
      print "Done visiting points. I guess I can't find any route?"
    end
  end
  nextPoint = targetPoint
  while (nextPoint ~= nil and nextPoint.previous ~= source) do
    nextPoint = nextPoint.previous
  end
  return nextPoint
end

function nextUnvisitedGridPoint()
  minDistance = math.huge
  nextPoint = nil
  for i, p in pairs(gridPoints) do
    if (not p.visited and p.distance < minDistance) then
      minDistance = p.distance
      nextPoint = p
    end
  end
  return nextPoint
end

function square(x)
  return x * x
end

function distanceToPoint(p)
  return math.sqrt(distanceSqToPoint(p))
end

function distanceSqToPoint(p)
  return square(p.x - ship:x()) + square(p.y - ship:y())
end

function bearingToPoint(p)
  return math.atan2(p.y - ship:y(), p.x - ship:x())
end

function normalRelativeAngle(x)
  while (x > math.pi) do
    x = x - (2 * math.pi)
  end
  while (x < -math.pi) do
    x = x + (2 * math.pi)
  end
  return x
end

function drawGrid(source)
  nextPoint = targetPoint
  for i, p in pairs(gridPoints) do
    for j, p2 in pairs(p.edges) do
      drawLine(p, p2, {r=50, g=50, b=50, a=255}, 1000000)
    end

    local color = {r=50, g=50, b=50}
    gfx:drawCircle(p.x, p.y, 3, color, 0, { }, 1000000)
  end
  while (nextPoint ~= nil and nextPoint.previous ~= source) do
    gfx:drawCircle(nextPoint.x, nextPoint.y, 3, {r=255, g=255, b=255}, 0, { },
                   1)
    if (nextPoint.previous ~= nil) then
      drawLine(nextPoint, nextPoint.previous, {r=255, g=255, b=255, a=255},
               1000000)
    end
    nextPoint = nextPoint.previous
  end
end

function drawPath(source, destination)
  nextPoint = targetPoint
  for i, p in pairs(gridPoints) do
    local color
    if (p == source) then
      gfx:drawCircle(p.x, p.y, 3, {r=255}, 0)
    elseif (p == destination) then
      gfx:drawCircle(p.x, p.y, 3, {r=255, g=255}, 0)
    end
  end
  while (nextPoint ~= nil and nextPoint.previous ~= source) do
    gfx:drawCircle(nextPoint.x, nextPoint.y, 3, {r=255, g=255, b=255}, 0)
    if (nextPoint.previous ~= nil) then
      drawLine(nextPoint, nextPoint.previous, {r=255, g=255, b=255, a=255}, 1)
    end
    nextPoint = nextPoint.previous
  end
end

function drawLine(p, p2, color, duration)
  gfx:drawLine(p.x, p.y, math.atan2(p2.y - p.y, p2.x - p.x),
      math.sqrt(square(p2.x - p.x) + square(p2.y - p.y)), 1, color, 0, { },
      duration)
end

Point = {}
Point.__index = Point

function Point.create(x, y)
  local p = {}
  setmetatable(p, Point)
  p.x = x
  p.y = y
  p.edges = {}
  p.distance = math.huge
  p.previous = nil
  p.visited = false
  return p
end

function Point:addEdge(p)
  table.insert(self.edges, p)
end

function Point:distanceSq(p)
  return square(p.x - self.x) + square(p.y - self.y)
end

function Point:pathExistsTo(p)
  return validPath(self.x, self.y, p.x, p.y)
end

function Point:isAdjacentTo(p)
  if (math.abs(self.x - p.x) <= gridSize
      and math.abs(self.y - p.y) <= gridSize) then
    return true
  end
  return false
end

Line = {}
Line.__index = Line

function Line.create(x1, y1, x2, y2)
  local l = {}
  setmetatable(l, Line)

  if (x1 == x2) then
    l.m = math.huge
    l.b = 0
    l.xMin = x1
    l.xMax = x1
  else
    l.m = (y2 - y1) / (x2 - x1)
    l.b = y1 - (l.m * x1)
    l.xMin = math.min(x1, x2)
    l.xMax = math.max(x1, x2)
  end
  l.yMin = math.min(y1, y2)
  l.yMax = math.max(y1, y2)
  l.x1 = x1
  l.y1 = y1
  l.x2 = x2
  l.y2 = y2
  l.inverseLine = nil
  return l
end

function Line:inverse()
  if (self.inverseLine == nil) then
    self.inverseLine = Line.create(self.y1, self.x1, self.y2, self.x2)
  end
  return self.inverseLine
end

function Line:intersects(l)
  if (l.xMin > self.xMax or self.xMin > l.xMax or l.yMin > self.yMax
      or self.yMin > l.yMax or self.m == l.m) then
    return false
  elseif (self.m == 0 and l.m == math.huge) then
    if (self.xMin < l.xMin and self.xMax > l.xMin and
        l.yMin < self.yMin and l.yMax > self.yMax) then
      return true
    else
      return false
    end
  elseif (self.m == math.huge or l.m == math.huge) then
    return self:inverse():intersects(l:inverse())
  else
    x = (l.b - self.b) / (self.m - l.m)
    y = self.m * x + self.b
    if (x >= self.xMin and x <= self.xMax) then
      return true
    else
      return false
    end
  end
end
