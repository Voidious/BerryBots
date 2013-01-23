cd luajit
make
cd ..
g++ bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp \
outputconsole.cpp linuxbasedir.cpp linuxresourcepath.cpp bbutil.cpp \
gfxmanager.cpp filemanager.cpp circle2d.cpp line2d.cpp point2d.cpp \
sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp gfxeventhandler.cpp \
rectangle.cpp stage.cpp cliprinthandler.cpp ./luajit/src/libluajit.a \
-I./luajit/src -I./stlsoft-1.9.116/include -I./sfml/include \
-I./wxWidgets/include -I./wxWidgets/gtk2-unicode-2.9 \
-D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -L./sfml-lib \
-L./wxWidgets-lib -pthread \
-Wl,-rpath,./wxWidgets-lib -lwx_gtk2u_xrc-2.9 \
-lwx_gtk2u_html-2.9 -lwx_gtk2u_qa-2.9 -lwx_gtk2u_adv-2.9 -lwx_gtk2u_core-2.9 \
-lwx_baseu_xml-2.9 -lwx_baseu_net-2.9 -lwx_baseu-2.9 -lsfml-graphics \
-lsfml-window -lsfml-system -ldl -o bbgui
cp ./scripts/bb_gui_linux.sh ./berrybots.sh
chmod 755 ./berrybots.sh
