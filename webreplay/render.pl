#! /usr/bin/perl
use strict;

my $filename = "/Users/pcupka/test.replay";

open(REPLAYFILE, "<" . $filename) || die "Can't open file";
binmode(REPLAYFILE); 

my $hexString = "";
$/ = \4;
my($numShips) = unpack 'i', <REPLAYFILE>;
print("Ships: " . $numShips . "\n");
$hexString .= sprintf("%x", $numShips) . ":";

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
  $hexString .= $name . ":";
}

$/ = \20;
my($z) = 0;
while (<REPLAYFILE>) {
  my($x, $y, $thrusterAngle, $thrusterForce, $energy) = unpack 'i i i i i';
  $hexString .= sprintf("%x", $x) . ":";
  $hexString .= sprintf("%x", $y) . ":";
  $hexString .= sprintf("%x", $thrusterAngle) . ":";
  $hexString .= sprintf("%x", $thrusterForce) . ":";
  $hexString .= sprintf("%x", $energy) . ":";
  print ("Ship " . $z . " position: " . ($x / 10) . ", " . ($y / 10)
      . " Thruster: " . ($thrusterAngle / 100) . " / "
      . ($thrusterForce / 100) . " Energy: " . ($energy / 10) . "\n");
  $z = ($z + 1) % $numShips;
}

close(REPLAYFILE);

chop $hexString;
print $hexString . "\n";
