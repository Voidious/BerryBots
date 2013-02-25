# Note: This is for building BerryBots to run single battles from the command
#       line, like the Raspberry Pi version.
cd luajit
make
cd ..
g++ bbsfmlmain.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp circle2d.cpp \
line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp \
gfxeventhandler.cpp rectangle.cpp stage.cpp cliprinthandler.cpp \
clipackagereporter.cpp dockitem.cpp dockshape.cpp docktext.cpp dockfader.cpp \
zipper.cpp guizipper.cpp \
./luajit/src/libluajit.a ./libarchive/libarchive.a \
/usr/lib/libz.dylib /usr/lib/libiconv.dylib \
-I./luajit/src -I./libarchive/include -I./stlsoft-1.9.116/include \
-I./sfml/include -L./sfml-lib -lsfml-graphics -lsfml-window -lsfml-system -ldl \
-pagezero_size 10000 -image_base 100000000 \
-o bbmain
cp ./scripts/bb_osx.sh ./berrybots.sh
chmod 755 ./berrybots.sh
