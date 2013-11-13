#!/usr/bin/perl
use CGI;

$basedir = "/home/ubuntu/berrybots";

@stages = ('battle1.lua', 'joust.lua');
@opponents = ('chaser.lua', 'jouster.lua', 'randombot.lua', 'wallhugger.lua',
              'basicbattler.lua');

$query = new CGI;

$stage = $query->param("stage");
$opponent = $query->param("opponent");

print "Content-type: text/html\n\n";

if (!isValidStage($stage)) {
  die("Invalid stage: " + $stage . "\n");
}

if (!isValidOpponent($opponent)) {
  die("Invalid opponent: " + $opponent . "\n");
}

chdir($basedir);
$s = `$basedir/berrybots stages/sample/$stage bots/sample/floatingduck.lua bots/sample/$opponent`;
$s =~ /\nSaved replay to: replays\/(.*)\n/;
$replayFilename = $1;

`cp $basedir/replays/$replayFilename /var/www/replays`;

print "<?xml version=\"1.0\" ?>\n<r>" . $replayFilename . "</r>\n";


sub isValidStage {
  my $stage = $_[0];
  my $valid = 0;
  foreach my $validStage (@stages) {
    if ($stage == $validStage) {
      $valid = 1;
      break;
    }
  }
  return $valid;
}

sub isValidOpponent {
  my $opponent = $_[0];
  my $valid = 0;
  foreach my $validOpponent (@opponents) {
    if ($opponent == $validOpponent) {
      $valid = 1;
      break;
    }
  }
  return $valid;
}
