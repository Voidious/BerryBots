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

##############################################################################
# BerryBots GUI common source files
SOURCES =  bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp
SOURCES += outputconsole.cpp relativebasedir.cpp linuxresourcepath.cpp
SOURCES += bbutil.cpp gfxmanager.cpp filemanager.cpp circle2d.cpp line2d.cpp 
SOURCES += point2d.cpp sensorhandler.cpp zone.cpp bbengine.cpp bblua.cpp
SOURCES += gfxeventhandler.cpp rectangle.cpp stage.cpp packagedialog.cpp
SOURCES += packageship.cpp packagestage.cpp dockitem.cpp dockshape.cpp
SOURCES += docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp menubarmaker.cpp
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
# Sources and flags for building on Mac OS X / Cocoa
OSX_EXTRA_SOURCES =  ./luajit/src/libluajit.a
OSX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a
OSX_EXTRA_SOURCES += /usr/lib/libz.dylib /usr/lib/libiconv.dylib

OSX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
OSX_CFLAGS += -I${SFML_PATH}/include `${WXWIDGETS_PATH}/wx-config --cflags`

OSX_LDFLAGS =  -L${SFML_BUILD_PATH}/lib `${WXWIDGETS_PATH}/wx-config --libs`
OSX_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl
OSX_LDFLAGS += -pagezero_size 10000 -image_base 100000000
##############################################################################


##############################################################################
# Sources and flags for building on Linux / GTK
LINUX_EXTRA_SOURCES =  ./luajit/src/libluajit.a
LINUX_EXTRA_SOURCES += ${LIBARCHIVE_PATH}/.libs/libarchive.a 

LINUX_CFLAGS =  -I./luajit/src -I./stlsoft-1.9.116/include -I${LIBARCHIVE_PATH}
LINUX_CFLAGS += -I${SFML_PATH}/include
LINUX_CFLAGS += `${WXWIDGETS_PATH}/wx-config --cflags`

LINUX_LDFLAGS =  -L${SFML_BUILD_PATH}/lib `${WXWIDGETS_PATH}/wx-config --libs`
LINUX_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system -ldl
##############################################################################


##############################################################################
# Sources and flags for building on Windows
WIN_EXTRA_SOURCES =  bbres.o .\luajit\src\lua51.dll
WIN_EXTRA_SOURCES += ${LIBARCHIVE_PATH}\build\libarchive\libarchive_static.a
WIN_EXTRA_SOURCES += ${WIN_ZLIB_PATH}\zlib1.dll

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
##############################################################################


##############################################################################
# Build targets
PLATS = rpi osx linux windows

none:
	@echo "Please do                       "
	@echo "   make PLATFORM                "
	@echo "where PLATFORM is one of these: "
	@echo "   $(PLATS) "

MAKE_LUAJIT=cd luajit; $(MAKE); cd ..
CLEAN_LUAJIT=cd luajit; $(MAKE) clean; cd ..

rpi:
	$(MAKE_LUAJIT)
	$(CC) ${RPI_SOURCES} ${RPI_CFLAGS} ${RPI_LDFLAGS} -o bbmain
	cp ./scripts/bb_rpi.sh ./berrybots.sh
	chmod 755 ./berrybots.sh

osx:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${OSX_EXTRA_SOURCES} ${OSX_CFLAGS} ${OSX_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_osx.sh ./berrybots.sh
	chmod 755 ./berrybots.sh

linux:
	$(MAKE_LUAJIT)
	$(CC) ${SOURCES} ${LINUX_EXTRA_SOURCES} ${LINUX_CFLAGS} ${LINUX_LDFLAGS} -o bbgui
	cp -r ${SFML_BUILD_PATH}/lib ./sfml-lib
	cp ./scripts/bb_gui_linux.sh ./berrybots.sh
	cp ./icon.iconset/icon_128x128.png .
	chmod 755 ./berrybots.sh

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

winclean:
	$(WIN_CLEAN_LUAJIT)
	del *o
	del *.dll
	del BerryBots.exe
##############################################################################
