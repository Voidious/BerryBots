cd luajit
mingw32-make
cd ..

windres --include-dir=C:\wxWidgets-2.9.4\include berrybots.rc bbres.o
copy icon.iconset\icon_32x32.png .

g++ bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp ^
outputconsole.cpp relativebasedir.cpp linuxresourcepath.cpp bbutil.cpp ^
gfxmanager.cpp filemanager.cpp circle2d.cpp line2d.cpp point2d.cpp ^
sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp gfxeventhandler.cpp ^
rectangle.cpp stage.cpp packagedialog.cpp packageship.cpp packagestage.cpp ^
dockitem.cpp dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp ^
menubarmaker.cpp bbres.o ^
.\luajit\src\lua51.dll .\libarchive\libarchive_static.a .\zlib1.dll ^
-I.\luajit\src -I.\libarchive\include -I.\stlsoft-1.9.116\include ^
-I.\sfml\include -L.\sfml-lib -lpthread ^
-mthreads -DHAVE_W32API_H -D__WXMSW__ -DUNICODE ^
-IC:\wxWidgets-2.9.4\lib\gcc_lib\mswu -IC:\wxWidgets-2.9.4\include ^
-Wno-ctor-dtor-privacy -pipe -fmessage-length=0 ^
-mthreads -LC:\wxWidgets-2.9.4\lib\gcc_lib ^
-lwxmsw29u_html -lwxmsw29u_adv -lwxmsw29u_core -lwxbase29u_xml ^
-lwxbase29u_net -lwxbase29u -lwxtiff -lwxjpeg -lwxpng -lwxzlib ^
-lwxregexu -lwxexpat -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwxregexu ^
-lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 ^
-ladvapi32 -lwsock32 -lsfml-graphics -lsfml-window -lsfml-system ^
-o BerryBots

copy .\sfml-lib\sfml-graphics-2.dll .
copy .\sfml-lib\sfml-system-2.dll .
copy .\sfml-lib\sfml-window-2.dll .
copy .\luajit\src\lua51.dll .

