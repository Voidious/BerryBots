#!/usr/bin/perl
use CGI;

$basedir = "/home/ubuntu/berrybots";
$maxCodeLength = 512 * 1024;

@stages = ('battle1.lua', 'joust.lua', 'maze2.lua', 'lasergallery.lua',
           'vortex.lua');
@opponents = ('chaser.lua', 'jouster.lua', 'randombot.lua', 'wallhugger.lua',
              'basicbattler.lua', 'floatingduck.lua', '<none>');

$query = new CGI;

$stage = $query->param("stage");
$opponent = $query->param("opponent");
$code = $query->param("code");

$filename = "player" . int(rand(100000000)) . ".lua";
open (OUTFILE, ">" . $basedir . "/bots/" . $filename);
print (OUTFILE $code);
close(OUTFILE);

print "Content-type: text/html\n\n";

if (!isValidStage($stage)) {
  die("Invalid stage: " . $stage . "\n");
}

if (!isValidOpponent($opponent)) {
  die("Invalid opponent: " . $opponent . "\n");
}

if (length($code) > $maxCodeLength) {
  die("Code too big: " . length($code));
}

$opponent = "sample/" . $opponent;
if ($opponent eq "sample/basicbattler.lua") {
  $opponent = "super" . $opponent;
}
if ($opponent =~ /<none>/) {
  $opponent = "";
} else {
  $opponent = " bots/" . $opponent;
}

chdir($basedir);
$s = `$basedir/berrybots stages/sample/$stage bots/$filename$opponent`;
$s =~ /\nSaved replay to: replays\/(.*)\n/;
$replayFilename = $1;
unlink($basedir . "/bots/" . $filename);

`cp $basedir/replays/$replayFilename /var/www/replays`;

$s =~ /^([\s\S]*)\nSaved replay to: replays/;
$errorLog = $1;
$errorLog = "\n" . $errorLog;
$errorLog =~ s/\nShip: [^:]+: /\n\n/g;
$errorLog =~ s/^\s+//g;

print "<?xml version=\"1.0\" ?>\n<replay>\n  <r>" . $replayFilename . "</r>\n"
    . "  <e><![CDATA[" . $errorLog . "]]></e>\n</replay>\n";

sub isValidStage {
  my $stage = $_[0];
  my $valid = 0;
  foreach my $validStage (@stages) {
    if ($stage eq $validStage) {
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
    if ($opponent eq $validOpponent) {
      $valid = 1;
      break;
    }
  }
  return $valid;
}
