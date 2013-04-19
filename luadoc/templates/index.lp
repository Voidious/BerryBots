<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>BerryBots API Docs</title>
    <link href='http://fonts.googleapis.com/css?family=Ubuntu:regular,bold' rel='stylesheet' type='text/css'>
    <link rel="shortcut icon" href="favicon.gif">
    <link rel="stylesheet" href="<%=luadoc.doclet.html.link("luadoc.css")%>" type="text/css" />
	<!--meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/-->
</head>

<body>
<div id="container">

<div id="main">

<div id="navigation">
<%=luadoc.doclet.html.include("menu.lp", { doc=doc })%>

</div> <!-- id="navigation" -->

<div id="content">

<div id="intro">
    <p><img src="icon_64x64.png" style="float: left; margin-right: 0.5em"> 
      BerryBots ships and stages are programmed in Lua 5.1. If you're new to
      Lua, check out the
      <a href="http://www.lua.org/manual/5.1/">Lua 5.1 Manual</a>,
      the <a href="http://lua-users.org/wiki/TutorialDirectory">Tutorial Directory</a>
      at the Lua-users wiki, or learn by example from the
      <a href="http://berrybots.com/wiki/index.php?title=Category:Sample_Bots">sample ships</a>
      and
      <a href="http://berrybots.com/wiki/index.php?title=Category:Sample_Stages">sample stages</a>.
    </p>
    <p>
      To create a ship or stage, your main Lua program must implement the
      functions of the
      <a href="<%=luadoc.doclet.html.module_link("ShipControl", doc)%>">ShipControl</a>
      or
      <a href="<%=luadoc.doclet.html.module_link("StageControl", doc)%>">StageControl</a>
      interface. These are called by BerryBots before, during, and after the
      match. The parameters passed to these functions allow your program to
      learn about and control all gameplay elements in BerryBots.
    </p>
    <p>
      The Game Runner API lets you write code to execute BerryBots matches and
      process the results. To write a Game Runner program, your main Lua program
      must implement the functions of the
      <a href="<%=luadoc.doclet.html.module_link("RunnerControl", doc)%>">RunnerControl</a>
      interface. The parameters passed to your Game Runner program allow it to
      specify input parameters, prompt a user for input, queue BerryBots
      matches, process match results, and read and write files to disk.
    </p>
    <p>
      For more information about programming BerryBots ships and stages, or to
      post about your own creations, visit the
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
