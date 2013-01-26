cd luajit
make
cd ..
g++ bbsfmlmain.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp circle2d.cpp ^
line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp ^
gfxeventhandler.cpp rectangle.cpp stage.cpp cliprinthandler.cpp ^
clipackagereporter.cpp ./luajit/src/lua51.dll -I./luajit/src ^
-I./stlsoft-1.9.116/include -I./sfml/include -L./sfml-lib -lsfml-graphics ^
-lsfml-window -lsfml-system -o bbmain
mkdir luajit-lib
copy .\luajit\src\lua51.dll .\luajit-lib
copy .\scripts\bb_windows.bat .\berrybots.bat
