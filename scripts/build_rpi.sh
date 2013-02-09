cd luajit
make
cd ..
g++ bbpimain.cpp bbengine.cpp stage.cpp bbutil.cpp bblua.cpp line2d.cpp \
point2d.cpp circle2d.cpp rectangle.cpp zone.cpp bbpigfx.cpp filemanager.cpp \
gfxeventhandler.cpp sensorhandler.cpp cliprinthandler.cpp \
clipackagereporter.cpp libshapes.c oglinit.c zipper.cpp tarzipper.cpp \
./luajit/src/libluajit.a -I./luajit/src -I./stlsoft-1.9.116/include \
-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -L/opt/vc/lib \
-lGLESv2 -ldl -o bbmain
cp ./scripts/bb_rpi.sh ./berrybots.sh
chmod 755 ./berrybots.sh
