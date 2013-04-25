#! /usr/bin/perl
use strict;

my $filename = "/Users/pcupka/test.replay";

open(REPLAYFILE, "<" . $filename) || die "Can't open file";
binmode(REPLAYFILE); 

my $hexString = "";
$/ = \4;
my($teams) = unpack 'i', <REPLAYFILE>;
print("Teams: " . $teams . "\n");
$hexString .= sprintf("%x", $teams) . ":";
$/ = \8;
my($z) = 0;
while (<REPLAYFILE>) {
  my($x, $y) = unpack 'i i';
  $hexString .= sprintf("%x", $x) . ":";
  $hexString .= sprintf("%x", $y) . ":";
  print ("Ship " . $z . " position: " . ($x / 10) . ", " . ($y / 10) . "\n");
  $z = ($z + 1) % $teams;
}

close(REPLAYFILE);

chop $hexString;
print $hexString . "\n";
