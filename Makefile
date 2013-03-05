##############################################################################
# Configure build parameters.
CC = g++

# Paths to external dependencies built from source for compiling BerryBots GUI
# on Mac / Linux / Windows.
SFML_PATH = /home/user/LaurentGomila-SFML-e750453
WXWIDGETS_PATH = /home/user/wxWidgets-2.9.4
LIBARCHIVE_PATH = /home/user/libarchive-3.1.2

# Modify this if you built SFML somewhere else.
SFML_BUILD_PATH = ${SFML_PATH}/build

# Ignore for *nix platforms
WIN_ZLIB_PATH = C:\zlib127-dll
WIN_SFML_BUILD_PATH = ${SFML_PATH}\build
##############################################################################

# You shouldn't have to edit anything below this line. #######################

VERSION = 1.1.0

##############################################################################
# BerryBots GUI common source files
SOURCES =  bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp
SOURCES += outputconsole.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp
SOURCES += circle2d.cpp line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp
SOURCES += bbengine.cpp bblua.cpp gfxeventhandler.cpp rectangle.cpp stage.cpp
SOURCES += packagedialog.cpp packageship.cpp packagestage.cpp dockitem.cpp
SOURCES += dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp
SOURCES += menubarmaker.cpp
##############################################################################


##############################################################################
# Sources and flags for building on Raspberry Pi (Raspbian "wheezy")
RPI_SOURCES =  bbpimain.cpp bbengine.cpp stage.cpp bbutil.cpp bblua.cpp
RPI_SOURCES += line2d.cpp point2d.cpp circle2d.cpp rectangle.cpp zone.cpp
RPI_SOURCES += bbpigfx.cpp filemanager.cpp gfxeventhandler.cpp sensorhandler.cpp
RPI_SOURCES += cliprinthandler.cpp clipackagereporter.cpp libshapes.c oglinit.c
RPI_SOURCES += zipper.cpp tarzipper.cpp ./luajit/src/libluajit.a

RPI_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I/opt/vc/include
RPI_CFLAGS += -I/opt/vc/include/interface/vcos/pthreads

RPI_LDFLAGS = -L/opt/vc/lib -lGLESv2 -ldl
##############################################################################


##############################################################################
# BerryBots CLI common source files
CLI_SOURCES =  bbsfmlmain.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp
CLI_SOURCES += circle2d.cpp line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp
CLI_SOURCES += bbengine.cpp bblua.cpp gfxeventhandler.cpp rectangle.cpp
CLI_SOURCES += stage.cpp cliprinthandler.cpp clipackagereporter.cpp dockitem.cpp
CLI_SOURCES += dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp
##############################################################################


##############################################################################
# Sources and flags for building on Mac OS X / Cocoa
OSX_EXTRA_SOURCES =  osxbasedir.mm osxcfg.m ResourcePath.mm
OSX_EXTRA_SOURCES += ./luajit/src/libluajit.a
OSX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a
OSX_EXTRA_SOURCES += /usr/lib/libz.dylib /usr/lib/libiconv.dylib

OSX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
OSX_CFLAGS += -I${SFML_PATH}/include `${WXWIDGETS_PATH}/wx-config --cflags`

OSX_LDFLAGS =  -L${SFML_BUILD_PATH}/lib `${WXWIDGETS_PATH}/wx-config --libs`
OSX_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl
OSX_LDFLAGS += -pagezero_size 10000 -image_base 100000000 -std=c99

OSXCLI_EXTRA_SOURCES = ./luajit/src/libluajit.a
OSXCLI_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a
OSXCLI_EXTRA_SOURCES += /usr/lib/libz.dylib /usr/lib/libiconv.dylib

OSXCLI_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
OSXCLI_CFLAGS += -I${SFML_PATH}/include

OSXCLI_LDFLAGS =  -L${SFML_BUILD_PATH}/lib
OSXCLI_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl
OSXCLI_LDFLAGS += -pagezero_size 10000 -image_base 100000000
##############################################################################


##############################################################################
# Sources and flags for building on Linux / GTK
LINUX_EXTRA_SOURCES =  relativebasedir.cpp relativerespath.cpp
LINUX_EXTRA_SOURCES += ./luajit/src/libluajit.a
LINUX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a

LINUX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
LINUX_CFLAGS += -I${SFML_PATH}/include
LINUX_CFLAGS += `${WXWIDGETS_PATH}/wx-config --cflags`

LINUX_LDFLAGS =  -L${SFML_BUILD_PATH}/lib `${WXWIDGETS_PATH}/wx-config --libs`
LINUX_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl

LINUXCLI_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include
LINUXCLI_CFLAGS += -I${LIBARCHIVE_PATH} -I${SFML_PATH}/include

LINUXCLI_LDFLAGS =  -L${SFML_BUILD_PATH}/lib
LINUXCLI_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl
##############################################################################


##############################################################################
# Sources and flags for building on Windows
WIN_EXTRA_SOURCES =  relativebasedir.cpp relativerespath.cpp bbres.o
WIN_EXTRA_SOURCES += .\luajit\src\lua51.dll ${WIN_ZLIB_PATH}\zlib1.dll
WIN_EXTRA_SOURCES += ${LIBARCHIVE_PATH}\build\libarchive\libarchive_static.a

WIN_CFLAGS =  -I.\luajit\src -I.\stlsoft-1.9.116\include -I${LIBARCHIVE_PATH}
WIN_CFLAGS += -I${SFML_PATH}\include 
WIN_CFLAGS += -mthreads -DHAVE_W32API_H -D__WXMSW__ -DUNICODE
WIN_CFLAGS += -I${WXWIDGETS_PATH}\lib\gcc_lib\mswu -I${WXWIDGETS_PATH}\include
WIN_CFLAGS += -Wno-ctor-dtor-privacy -pipe -fmessage-length=0

WIN_LDFLAGS =  -L${WIN_SFML_BUILD_PATH}\lib -lpthread
WIN_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
WIN_LDFLAGS += -mthreads -L${WXWIDGETS_PATH}\lib\gcc_lib
WIN_LDFLAGS += -lwxmsw29u_html -lwxmsw29u_adv -lwxmsw29u_core -lwxbase29u_xml
WIN_LDFLAGS += -lwxbase29u_net -lwxbase29u -lwxtiff -lwxjpeg -lwxpng -lwxzlib
WIN_LDFLAGS += -lwxregexu -lwxexpat -lkernel32 -luser32 -lgdi32 -lcomdlg32
WIN_LDFLAGS += -lwxregexu -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32
WIN_LDFLAGS += -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32

WINCLI_CFLAGS =  -I.\luajit\src -I.\stlsoft-1.9.116\include -I${LIBARCHIVE_PATH}
WINCLI_CFLAGS += -I${SFML_PATH}\include 

WINCLI_LDFLAGS =  -L${WIN_SFML_BUILD_PATH}\lib -lpthread
WINCLI_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
##############################################################################


##############################################################################
# Build targets
GUIPLATS = osx linux windows
CLIPLATS = rpi osxcli linuxcli wincli

none:
	@echo "Please do                       "
	@echo "  make PLATFORM                 "
	@echo "where PLATFORM is one of these: "
	@echo "  GUI: $(GUIPLATS)     "
	@echo "  CLI: $(CLIPLATS)     "

MAKE_LUAJIT=cd luajit; $(MAKE); cd ..
CLEAN_LUAJIT=cd luajit; $(MAKE) clean; cd ..

rpi:
	$(MAKE_LUAJIT)
	$(CC) ${RPI_SOURCES} ${RPI_CFLAGS} ${RPI_LDFLAGS} -o bbmain
	cp ./scripts/bb_rpi.sh ./berrybots.sh
	chmod 755 ./berrybots.sh
	@echo "==== Successfully built BerryBots $(VERSION) ===="

osx:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${OSX_EXTRA_SOURCES} ${OSX_CFLAGS} ${OSX_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_osx.sh ./berrybots.sh
	chmod 755 ./berrybots.sh
	@echo "==== Successfully built BerryBots $(VERSION) ===="

osxcli:
	$(MAKE_LUAJIT)
	$(CC) ${CLI_SOURCES} ${OSXCLI_EXTRA_SOURCES} ${OSXCLI_CFLAGS} ${OSXCLI_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_osx.sh ./berrybots.sh
	chmod 755 ./berrybots.sh
	@echo "==== Successfully built BerryBots $(VERSION) ===="

linux:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${LINUX_EXTRA_SOURCES} ${LINUX_CFLAGS} ${LINUX_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_linux.sh ./berrybots.sh
	cp ./icon.iconset/icon_128x128.png .
	chmod 755 ./berrybots.sh
	@echo "==== Successfully built BerryBots $(VERSION) ===="

linuxcli:
	$(MAKE_LUAJIT)
	$(CC) ${CLI_SOURCES} ${LINUX_EXTRA_SOURCES} ${LINUXCLI_CFLAGS} ${LINUXCLI_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_linux.sh ./berrybots.sh
	cp ./icon.iconset/icon_128x128.png .
	chmod 755 ./berrybots.sh
	@echo "==== Successfully built BerryBots $(VERSION) ===="

clean:
	$(CLEAN_LUAJIT)
	rm -rf *o sfml-lib bbgui berrybots.sh

WIN_MAKE_LUAJIT=cd luajit && $(MAKE) && cd ..
WIN_CLEAN_LUAJIT=cd luajit && $(MAKE) clean && cd ..

windows:
	windres --include-dir=${WXWIDGETS_PATH}\include berrybots.rc bbres.o
	copy icon.iconset\icon_32x32.png .
	$(WIN_MAKE_LUAJIT)	
	$(CC) ${SOURCES} ${WIN_EXTRA_SOURCES} ${WIN_CFLAGS} ${WIN_LDFLAGS} -o BerryBots
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-graphics-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-system-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-window-2.dll .
	copy ${WIN_ZLIB_PATH}\zlib1.dll .
	copy .\luajit\src\lua51.dll .
	@echo "==== Successfully built BerryBots $(VERSION) ===="

wincli:
	copy icon.iconset\icon_32x32.png .
	$(WIN_MAKE_LUAJIT)	
	$(CC) ${CLI_SOURCES} ${WIN_EXTRA_SOURCES} ${WINCLI_CFLAGS} ${WINCLI_LDFLAGS} -o berrybots
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-graphics-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-system-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-window-2.dll .
	copy ${WIN_ZLIB_PATH}\zlib1.dll .
	copy .\luajit\src\lua51.dll .
	@echo "==== Successfully built BerryBots $(VERSION) ===="

winclean:
	$(WIN_CLEAN_LUAJIT)
	del *o
	del *.dll
	del BerryBots.exe
##############################################################################
