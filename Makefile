CC=g++

# Paths to external dependencies built from source
SFML_PATH = /home/user/LaurentGomila-SFML-e750453
WXWIDGETS_PATH = /home/user/wxWidgets-2.9.4
LIBARCHIVE_PATH = /home/user/libarchive-3.1.1

# BerryBots source files
SOURCES =  bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp
SOURCES += outputconsole.cpp relativebasedir.cpp linuxresourcepath.cpp
SOURCES += bbutil.cpp gfxmanager.cpp filemanager.cpp circle2d.cpp line2d.cpp 
SOURCES += point2d.cpp sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp
SOURCES += gfxeventhandler.cpp rectangle.cpp stage.cpp packagedialog.cpp
SOURCES += packageship.cpp packagestage.cpp dockitem.cpp dockshape.cpp
SOURCES += docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp menubarmaker.cpp


##############################################################################
# Sources and flags for building on Mac OS X / Cocoa
OSX_EXTRA_SOURCES =  ./luajit/src/libluajit.a
OSX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a
OSX_EXTRA_SOURCES += /usr/lib/libz.dylib /usr/lib/libiconv.dylib

OSX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
OSX_CFLAGS += -I${SFML_PATH}/include
OSX_CFLAGS += -I${WXWIDGETS_PATH}/lib/wx/include/osx_cocoa-unicode-2.9
OSX_CFLAGS += -I${WXWIDGETS_PATH}/include -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL
OSX_CFLAGS += -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__

OSX_LDFLAGS =  -L${SFML_PATH}/build/lib -L${WXWIDGETS_PATH}/lib -framework IOKit
OSX_LDFLAGS += -framework Carbon -framework Cocoa -framework AudioToolbox
OSX_LDFLAGS += -framework System -framework OpenGL -lwx_osx_cocoau_xrc-2.9
OSX_LDFLAGS += -lwx_osx_cocoau_webview-2.9 -lwx_osx_cocoau_html-2.9
OSX_LDFLAGS += -lwx_osx_cocoau_qa-2.9 -lwx_osx_cocoau_adv-2.9
OSX_LDFLAGS += -lwx_osx_cocoau_core-2.9 -lwx_baseu_xml-2.9 -lwx_baseu_net-2.9
OSX_LDFLAGS += -lwx_baseu-2.9 -lsfml-graphics -lsfml-window -lsfml-system -ldl
OSX_LDFLAGS += -pagezero_size 10000 -image_base 100000000
##############################################################################


##############################################################################
# Sources and flags for building on Linux / GTK
LINUX_EXTRA_SOURCES =  ./luajit/src/libluajit.a
LINUX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a 

LINUX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
LINUX_CFLAGS += -I${SFML_PATH}/include
LINUX_CFLAGS += -I${WXWIDGETS_PATH}/lib/wx/include/gtk2-unicode-2.9
LINUX_CFLAGS += -I${WXWIDGETS_PATH}/include -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL
LINUX_CFLAGS += -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__

LINUX_LDFLAGS =  -L${SFML_PATH}/build/lib -L${WXWIDGETS_PATH}/lib -pthread
LINUX_LDFLAGS += -Wl,-rpath,${WXWIDGETS_PATH}/lib -lwx_gtk2u_xrc-2.9
LINUX_LDFLAGS += -lwx_gtk2u_html-2.9 -lwx_gtk2u_qa-2.9 -lwx_gtk2u_adv-2.9
LINUX_LDFLAGS += -lwx_gtk2u_core-2.9 -lwx_baseu_xml-2.9 -lwx_baseu_net-2.9
LINUX_LDFLAGS += -lwx_baseu-2.9 -lsfml-graphics -lsfml-window -lsfml-system -ldl
##############################################################################


##############################################################################
# Build targets
PLATS = osx linux windows

MAKE_LUAJIT=cd luajit; make; cd ..
CLEAN_LUAJIT=cd luajit; make clean; cd ..

none:
	@echo "Please do"
	@echo "   make PLATFORM"
	@echo "where PLATFORM is one of these:"
	@echo "   $(PLATS)"

osx:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${OSX_EXTRA_SOURCES} ${OSX_CFLAGS} ${OSX_LDFLAGS} -o bbgui
	cp ./scripts/bb_gui_osx.sh ./berrybots.sh
	chmod 755 ./berrybots.sh

linux:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${LINUX_EXTRA_SOURCES} ${LINUX_CFLAGS} ${LINUX_LDFLAGS} -o bbgui
	cp ./scripts/bb_gui_linux.sh ./berrybots.sh
	cp ./icon.iconset/icon_128x128.png .
	chmod 755 ./berrybots.sh

clean:
	$(CLEAN_LUAJIT)
	rm -rf *o bbgui
##############################################################################
