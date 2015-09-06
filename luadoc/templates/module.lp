<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>BerryBots API: <%=module_doc.name%></title>
<link href='http://fonts.googleapis.com/css?family=Ubuntu:regular,bold' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Ubuntu:regular,bold' rel='stylesheet' type='text/css'>
<link rel="stylesheet" href="<%=luadoc.doclet.html.link('luadoc.css', 'modules/'..module_doc.name)%>" type="text/css" />

<!-- Favicons generated using: http://realfavicongenerator.net/ - Thanks! -->
<link rel="apple-touch-icon" sizes="57x57" href="../favicon/apple-touch-icon-57x57.png">
<link rel="apple-touch-icon" sizes="60x60" href="../favicon/apple-touch-icon-60x60.png">
<link rel="apple-touch-icon" sizes="72x72" href="../favicon/apple-touch-icon-72x72.png">
<link rel="apple-touch-icon" sizes="76x76" href="../favicon/apple-touch-icon-76x76.png">
<link rel="apple-touch-icon" sizes="114x114" href="../favicon/apple-touch-icon-114x114.png">
<link rel="apple-touch-icon" sizes="120x120" href="../favicon/apple-touch-icon-120x120.png">
<link rel="apple-touch-icon" sizes="144x144" href="../favicon/apple-touch-icon-144x144.png">
<link rel="apple-touch-icon" sizes="152x152" href="../favicon/apple-touch-icon-152x152.png">
<link rel="apple-touch-icon" sizes="180x180" href="../favicon/apple-touch-icon-180x180.png">
<link rel="icon" type="image/png" href="../favicon/favicon-32x32.png" sizes="32x32">
<link rel="icon" type="image/png" href="../favicon/android-chrome-192x192.png" sizes="192x192">
<link rel="icon" type="image/png" href="../favicon/favicon-96x96.png" sizes="96x96">
<link rel="icon" type="image/png" href="../favicon/favicon-16x16.png" sizes="16x16">
<link rel="manifest" href="../favicon/manifest.json">
<link rel="shortcut icon" href="../favicon/favicon.ico">
<meta name="msapplication-TileColor" content="#000000">
<meta name="msapplication-TileImage" content="../favicon/mstile-144x144.png">
<meta name="msapplication-config" content="../favicon/browserconfig.xml">
<meta name="theme-color" content="#000000">

</head>

<body>
<div id="container">

<div id="main">

<div id="navigation">
<%=luadoc.doclet.html.include("menu.lp", { doc=doc, module_doc=module_doc })%>

</div><!-- id="navigation" -->

<div id="content">

<h1>Module <code><%=module_doc.name%></code></h1>

<p><%=module_doc.description%></p>
<%if module_doc.author then%>
<p><b><%= #module_doc.author>1 and "Authors" or "Author" %>:</b>
<table class="authors_list">
<%for _, author in ipairs(module_doc.author) do%>
	<tr><td class="name"><%= author %></td></tr>
<%end%>
</table>
</p>
<%end%>
<%if module_doc.copyright then%>
<p>Copyright&copy; <%=module_doc.copyright%></p>
<%end%>
<%if module_doc.release then%>
<p><small><b>Release:</b> <%=module_doc.release%></small></p>
<%end%>

<%if #module_doc.functions > 0 then%>
<h2>Functions</h2>
<table class="function_list">
<%for _, func_name in ipairs(module_doc.functions) do
  local func_data = module_doc.functions[func_name]%>
	<tr>
	<td class="name" nowrap><%=func_data.private and "local " or ""%><a href="#<%=func_name%>"><%=func_name%></a>&nbsp;(<%=table.concat(module_doc.functions[func_name].param, ", ")%>)</td>
	<td class="summary"><%=module_doc.functions[func_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<%if #module_doc.tables > 0 then%>
<h2>Tables</h2>
<table class="table_list">
<%for _, tab_name in ipairs(module_doc.tables) do%>
	<tr>
	<td class="name" nowrap><a href="#<%=tab_name%>"><%=tab_name%></a></td>
	<td class="summary"><%=module_doc.tables[tab_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<br/>
<br/>


<%if #module_doc.functions > 0 then%>
<h2><a name="functions"></a>Functions</h2>
<dl class="function">
<%for _, func_name in ipairs(module_doc.functions) do%>
<%=luadoc.doclet.html.include("function.lp", { doc=doc, module_doc=module_doc, func=module_doc.functions[func_name] })%>
<%end%>
</dl>
<%end%>


<%if #module_doc.tables > 0 then%>
<h2><a name="tables"></a>Tables</h2>
<dl class="table">
<%for _, tab_name in ipairs(module_doc.tables) do%>
<%=luadoc.doclet.html.include("table.lp", { doc=doc, module_doc=module_doc, tab=module_doc.tables[tab_name] })%>
<%end%>
</dl>
<%end%>


</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
</div> <!-- id="about" -->

</div> <!-- id="container" -->	
</body>
</html>
