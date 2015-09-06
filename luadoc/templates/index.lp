<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>BerryBots API Docs</title>
<link href='http://fonts.googleapis.com/css?family=Ubuntu:regular,bold' rel='stylesheet' type='text/css'>
<link rel="shortcut icon" href="favicon.gif">
<link rel="stylesheet" href="<%=luadoc.doclet.html.link("luadoc.css")%>" type="text/css" />

<!-- Favicons generated using: http://realfavicongenerator.net/ - Thanks! -->
<link rel="apple-touch-icon" sizes="57x57" href="favicon/apple-touch-icon-57x57.png">
<link rel="apple-touch-icon" sizes="60x60" href="favicon/apple-touch-icon-60x60.png">
<link rel="apple-touch-icon" sizes="72x72" href="favicon/apple-touch-icon-72x72.png">
<link rel="apple-touch-icon" sizes="76x76" href="favicon/apple-touch-icon-76x76.png">
<link rel="apple-touch-icon" sizes="114x114" href="favicon/apple-touch-icon-114x114.png">
<link rel="apple-touch-icon" sizes="120x120" href="favicon/apple-touch-icon-120x120.png">
<link rel="apple-touch-icon" sizes="144x144" href="favicon/apple-touch-icon-144x144.png">
<link rel="apple-touch-icon" sizes="152x152" href="favicon/apple-touch-icon-152x152.png">
<link rel="apple-touch-icon" sizes="180x180" href="favicon/apple-touch-icon-180x180.png">
<link rel="icon" type="image/png" href="favicon/favicon-32x32.png" sizes="32x32">
<link rel="icon" type="image/png" href="favicon/android-chrome-192x192.png" sizes="192x192">
<link rel="icon" type="image/png" href="favicon/favicon-96x96.png" sizes="96x96">
<link rel="icon" type="image/png" href="favicon/favicon-16x16.png" sizes="16x16">
<link rel="manifest" href="favicon/manifest.json">
<link rel="shortcut icon" href="favicon/favicon.ico">
<meta name="msapplication-TileColor" content="#000000">
<meta name="msapplication-TileImage" content="favicon/mstile-144x144.png">
<meta name="msapplication-config" content="favicon/browserconfig.xml">
<meta name="theme-color" content="#000000">

</head>

<body>
<div id="container">

<div id="main">

<div id="navigation">
<%=luadoc.doclet.html.include("menu.lp", { doc=doc })%>

</div> <!-- id="navigation" -->

<div id="content">

<div id="intro">
    <p><img src="icon_128x128.png" width="64" height="64" style="float: left; margin-right: 0.5em">
      BerryBots programs are written in Lua. If you're new to Lua, check out the
      <a href="http://www.lua.org/manual/5.1/">Lua 5.1 Manual</a>, the
      <a href="http://lua-users.org/wiki/TutorialDirectory">Tutorial Directory</a>
      at the Lua-users wiki, or learn by example from the
      <a href="http://berrybots.com/wiki/index.php?title=Category:Samples">BerryBots sample programs</a>.
    </p>
    <p>
      To create a ship or stage, your Lua program must implement the functions
      of the
      <a href="<%=luadoc.doclet.html.module_link("ShipControl", doc)%>">ShipControl</a>
      or
      <a href="<%=luadoc.doclet.html.module_link("StageControl", doc)%>">StageControl</a>
      interface. These functions are called by BerryBots before, during, and
      after the match. The parameters passed to them allow your program to
      learn about and control the gameplay elements in BerryBots.
    </p>
    <p>
      The Game Runner API lets you write code to execute BerryBots matches,
      process the results, and (optionally) save replays. To write a Game
      Runner, your Lua program must implement the functions of the
      <a href="<%=luadoc.doclet.html.module_link("RunnerControl", doc)%>">RunnerControl</a>
      interface. The parameters passed to your program allow it to specify input
      parameters, prompt a user for input, queue BerryBots matches, process
      match results, save replays, and read and write files to disk.
    </p>
    <p>
      For more information about programming for BerryBots, or to post about
      your own creations, visit the
      <a href="http://berrybots.com/wiki">BerryBots Wiki</a>.
    </p>
</div>

<%if not options.nomodules and #doc.modules > 0 then%>
<h2>Ship / Stage Modules</h2>
<table class="module_list">
<!--<tr><td colspan="2">Modules</td></tr>-->
<%for _, modulename in ipairs(doc.modules) do
    if (modulename ~= "MatchRunner" and modulename ~= "RunnerControl"
        and modulename ~= "RunnerFiles" and modulename ~= "RunnerForm") then%>
	<tr>
		<td class="name"><a href="<%=luadoc.doclet.html.module_link(modulename, doc)%>"><%=modulename%></a></td>
		<td class="summary"><%=doc.modules[modulename].summary%></td>
	</tr>
<%  end
  end%>
</table>
<%end%>


<%if not options.nomodules and #doc.modules > 0 then%>
<h2>Game Runner Modules</h2>
<table class="module_list">
<!--<tr><td colspan="2">Modules</td></tr>-->
<%for _, modulename in ipairs(doc.modules) do
    if (modulename == "MatchRunner" or modulename == "RunnerControl"
        or modulename == "RunnerFiles" or modulename == "RunnerForm") then%>
	<tr>
		<td class="name"><a href="<%=luadoc.doclet.html.module_link(modulename, doc)%>"><%=modulename%></a></td>
		<td class="summary"><%=doc.modules[modulename].summary%></td>
	</tr>
<%  end
  end%>
</table>
<%end%>



<%if not options.nofiles and #doc.files > 0 then%>
<h2>Files</h2>
<table class="file_list">
<!--<tr><td colspan="2">Files</td></tr>-->
<%for _, filepath in ipairs(doc.files) do%>
	<tr>
		<td class="name"><a href="<%=luadoc.doclet.html.file_link(filepath)%>"><%=filepath%></a></td>
		<td class="summary"></td>
	</tr>
<%end%>
</table>
<%end%>

</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
</div> <!-- id="about" -->

</div> <!-- id="container" -->	
</body>
</html>
