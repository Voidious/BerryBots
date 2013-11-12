#!/usr/bin/perl

chdir('/home/admin/berrybots');
$s = `/home/admin/berrybots/berrybots stages/sample/arcadeshooter.lua bots/sample/shooterbot.lua`;
$s =~ /\nSaved replay to: replays\/(.*)\n/;
$replayFilename = $1;

`cp /home/admin/berrybots/replays/$replayFilename /var/www/replays`;

print "Content-type: text/html\n\n";
print "<?xml version=\"1.0\" ?>\n<r>" . $replayFilename . "</r>\n";
