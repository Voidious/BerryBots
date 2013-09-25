#! /usr/bin/perl
use strict;

# Replay format:
#
# Stage size:  (2)
# width | height
#
# Wall:  (4)
# left | bottom | width | height
#
# Zone:  (4)
# left | bottom | width | height
#
# Ship properties:  (variable)
# ship R | G | B | laser R | G | B | thruster R | G | B | nameLength | <name>
#
# Ship add:  (2)
# ship index | time
#
# Ship remove:  (2)
# ship index | time
#
# Ship tick:  (5)
# x * 10 | y * 10 | thruster angle * 100 | force * 100 | energy * 10
#
# Laser start:  (6)
# laser ID | ship index | fire time | x * 10 | y * 10 | heading * 100
#
# Laser end:  (2)
# laser ID | end time
#
# Laser spark:  (6)
# ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100
#
# Torpedo start:  (6)
# torpedo ID | ship index | fire time | x * 10 | y * 10 | heading * 100 
#
# Torpedo end:  (2)
# torpedo ID | end time
#
# Torpedo blast:  (3)
# time | x * 10 | y * 10
#
# Torpedo debris:  (7)
# ship index | time | x * 10 | y * 10 | dx * 100 | dy * 100 | parts
#
# Ship destroy:  (4)
# ship index | time | x * 10 | y * 10
#
# Text:  (variable)
# time | textLength | text | x * 10 | y * 10 | size | text R | G | B | A | duration
#
# Complete file:
# | replay version
# | stage width | stage height | num walls | <walls> | num zones | <zones>
# | num ships | <ship properties> | num ship adds | <ship adds>
# | num ship removes | <ship removes> | num ship ticks | <ship ticks>
# | num laser starts | <laser starts> | num laser ends | <laser ends>
# | num laser sparks | <laser sparks>
# | num torpedo starts | <torpedo starts> | num torpedo ends | <torpedo ends>
# | num torpedo blasts | <torpedo blasts> | num torpedo debris | <torpedo debris>
# | num ship destroys | <ship destroys> | num texts | <texts>

my $filename = "/Users/pcupka/test.bbr";

open(REPLAYFILE, "<" . $filename) || die "Can't open file";
binmode(REPLAYFILE); 
$/ = \4;

my $hexString = "";

my $version = unpack 'i', <REPLAYFILE>;
hexAppend($version);
print("Replay version: " . $version . "\n");

my $width = unpack 'i', <REPLAYFILE>;
hexAppend($width);
my $height = unpack 'i', <REPLAYFILE>;
hexAppend($height);
print("Stage size: " . $width . ", " . $height . "\n");

my $numWalls = processDataBlock(4);
print("Walls: " . $numWalls . "\n");

my $numZones = processDataBlock(4);
print("Zones: " . $numZones . "\n");

my $numShips = unpack 'i', <REPLAYFILE>;
hexAppend($numShips);
print("Ships: " . $numShips . "\n");
for (my $x = 0; $x < $numShips; $x++) {
  for (my $y = 0; $y < 3; $y++) {
    my $r = unpack 'i', <REPLAYFILE>;
    my $g = unpack 'i', <REPLAYFILE>;
    my $b = unpack 'i', <REPLAYFILE>;
    my $rgbValue = sprintf("#%02x%02x%02x", $r, $g, $b);
    print("Ship " . $x . " color " . ($y + 1) . ": " . $rgbValue . "\n");
    $hexString .= $rgbValue . ":";
  }
  my $nameLength = unpack 'i', <REPLAYFILE>;
  my $name = "";
  for (my $y = 0; $y < $nameLength; $y++) {
    $name .= chr(unpack 'i', <REPLAYFILE>);
  }
  print("Ship " . $x . " name: " . $name . "\n");
  $hexString .= escapeColons($name) . ":";
}

my $numShipAdds = processDataBlock(2);
print("Ship adds: " . $numShipAdds . "\n");

my $numShipRemoves = processDataBlock(2);
print("Ship removes: " . $numShipRemoves . "\n");

my $numShipTicks = processDataBlock(5);
print("Ship ticks: " . $numShipTicks . "\n");

my $numLaserStarts = processDataBlock(6);
print("Laser starts: " . $numLaserStarts . "\n");

my $numLaserEnds = processDataBlock(2);
print("Laser ends: " . $numLaserEnds . "\n");

my $numLaserSparks = processDataBlock(6);
print("Laser sparks: " . $numLaserSparks . "\n");

my $numTorpedoStarts = processDataBlock(6);
print("Torpedo starts: " . $numTorpedoStarts . "\n");

my $numTorpedoEnds = processDataBlock(2);
print("Torpedo ends: " . $numTorpedoEnds . "\n");

my $numTorpedoBlasts = processDataBlock(3);
print("Torpedo blasts: " . $numTorpedoBlasts . "\n");

my $numTorpedoDebris = processDataBlock(7);
print("Torpedo debris: " . $numTorpedoDebris . "\n");

my $numShipDestroys = processDataBlock(4);
print("Ship destroys: " . $numShipDestroys . "\n");

my $numTexts = unpack 'i', <REPLAYFILE>;
hexAppend($numTexts);
print("Texts: " . $numTexts . "\n");
for (my $x = 0; $x < $numTexts; $x++) {
  my $time = unpack 'i', <REPLAYFILE>;
  hexAppend($time);

  my $textLength = unpack 'i', <REPLAYFILE>;
  my $text = "";
  for (my $y = 0; $y < $textLength; $y++) {
    $text .= chr(unpack 'i', <REPLAYFILE>);
  }
  $hexString .= escapeColons($text) . ":";

  for (my $y = 0; $y < 3; $y++) {
    my $z = unpack 'i', <REPLAYFILE>;
    hexAppend($z);
  }

  my $r = unpack 'i', <REPLAYFILE>;
  my $g = unpack 'i', <REPLAYFILE>;
  my $b = unpack 'i', <REPLAYFILE>;
  my $rgbValue = sprintf("#%02x%02x%02x", $r, $g, $b);
  $hexString .= $rgbValue . ":";

  for (my $y = 0; $y < 2; $y++) {
    my $z = unpack 'i', <REPLAYFILE>;
    hexAppend($z);
  }
}

close(REPLAYFILE);

chop $hexString;
print $hexString . "\n";

sub processDataBlock {
  my $blockSize = $_[0];
  my $numBlocks = unpack 'i', <REPLAYFILE>;
  hexAppend($numBlocks);
  for (my $x = 0; $x < $numBlocks; $x++) {
    for (my $y = 0; $y < $blockSize; $y++) {
      my $z = unpack 'i', <REPLAYFILE>;
      hexAppend($z);
    }
  }
  return $numBlocks;
}

sub hexAppend {
  my $i = $_[0];
  if ($i >= 0) {
    $hexString .= sprintf("%x", $i) . ":";
  } else {
    $hexString .= "-" . sprintf("%x", $i * -1) . ":";
  }
}

sub escapeColons {
  my $s = $_[0];
  $s =~ s/:/\\\\:/g;
  return $s;
}
