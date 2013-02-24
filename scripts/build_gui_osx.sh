cd luajit
make
cd ..
g++ bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp \
outputconsole.cpp relativebasedir.cpp linuxresourcepath.cpp bbutil.cpp \
gfxmanager.cpp filemanager.cpp circle2d.cpp line2d.cpp point2d.cpp \
sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp gfxeventhandler.cpp \
rectangle.cpp stage.cpp packagedialog.cpp packageship.cpp packagestage.cpp \
dockitem.cpp dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp \
menubarmaker.cpp \
./luajit/src/libluajit.a ./libarchive/libarchive.a \
/usr/lib/libz.dylib /usr/lib/libiconv.dylib \
-I./luajit/src -I./libarchive/include -I./stlsoft-1.9.116/include \
-I./sfml/include -L./sfml-lib \
-I./wxWidgets/osx_cocoa-unicode-2.9 -I./wxWidgets/include \
-D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__ \
-L./wxWidgets-lib -framework IOKit -framework Carbon -framework Cocoa \
-framework AudioToolbox -framework System -framework OpenGL \
-lwx_osx_cocoau_xrc-2.9 -lwx_osx_cocoau_webview-2.9 -lwx_osx_cocoau_html-2.9 \
-lwx_osx_cocoau_qa-2.9 -lwx_osx_cocoau_adv-2.9 -lwx_osx_cocoau_core-2.9 \
-lwx_baseu_xml-2.9 -lwx_baseu_net-2.9 -lwx_baseu-2.9  \
-lsfml-graphics -lsfml-window -lsfml-system -ldl \
-pagezero_size 10000 -image_base 100000000 \
-o bbgui
cp ./scripts/bb_gui_osx.sh ./berrybots.sh
cp ./icon.iconset/icon_128x128.png .
chmod 755 ./berrybots.sh
