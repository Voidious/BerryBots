<%
if module_doc then
  from = "modules/"..module_doc.name
elseif file_doc then
  from = "files/.."..file_doc.name
else
  from = ""
end
%>

<h1>BerryBots Lua API</h1>
<ul>
  <%if not module_doc and not file_doc then%>
  <li><strong>Index</strong></li>
  <%else%>
  <li><a href="<%=luadoc.doclet.html.link("index.html", from)%>">Index</a></li>
  <%end%>
</ul>


<!-- Ship / stage module list -->
<%if not options.nomodules and #doc.modules > 0 then%>
<h1>Ship / Stage Modules</h1>
<ul>
<%for _, modulename in ipairs(doc.modules) do
  if (modulename ~= "MatchRunner" and modulename ~= "RunnerControl"
      and modulename ~= "RunnerFiles" and modulename ~= "RunnerForm") then
    if module_doc and module_doc.name == modulename then%>
  <li><strong><%=modulename%></strong></li>
  <%else%>
  <li>
    <a href="<%=luadoc.doclet.html.module_link(modulename, doc, from)%>"><%=modulename%></a>
  </li>
<%  end
  end
end%>
</ul>
<%end%>


<!-- Game Runner module list -->
<%if not options.nomodules and #doc.modules > 0 then%>
<h1>Game Runner Modules</h1>
<ul>
<%for _, modulename in ipairs(doc.modules) do
  if (modulename == "MatchRunner" or modulename == "RunnerControl"
      or modulename == "RunnerFiles" or modulename == "RunnerForm") then
    if module_doc and module_doc.name == modulename then%>
  <li><strong><%=modulename%></strong></li>
  <%else%>
  <li>
    <a href="<%=luadoc.doclet.html.module_link(modulename, doc, from)%>"><%=modulename%></a>
  </li>
<%  end
  end
end%>
</ul>
<%end%>


<!-- File list -->
<%if not options.nofiles and #doc.files > 0 then%>
<h1>Files</h1>
<ul>
<%for _, filepath in ipairs(doc.files) do
  if file_doc and file_doc.name == filepath then%>
  <li><strong><%=filepath%></strong></li>
  <%else%>
  <li>
    <a href="<%=luadoc.doclet.html.file_link(filepath, from)%>"><%=filepath%></a>
  </li>
<%  end
end%>
</ul>
<%end%>


<h1>Helpful Links</h1>
<ul>
    <li><a href="http://www.lua.org/manual/5.1/">Lua 5.1 Manual</a></li>
    <li><a href="http://lua-users.org/wiki/TutorialDirectory">Lua-users Tutorial Directory</a></li>
    <li><a href="http://berrybots.com/wiki/index.php?title=Category:Sample_Bots">Sample Ships</a>
    <li><a href="http://berrybots.com/wiki/index.php?title=Category:Sample_Stages">Sample Stages</a>
    <li><a href="http://berrybots.com/specs.html">BerryBots Game Specs</a></li>
    <li><a href="http://berrybots.com/wiki">BerryBots Wiki</a></li>
</ul>
