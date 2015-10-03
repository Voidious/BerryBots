##############################################################################
# This Makefile is only here for Windows. On *nix, you should use ./configure,
# which will overwrite the Makefile, then run "make" and (optionally)
# "make install".
##############################################################################
# Configure build parameters.
CC = g++

SFML_PATH = C:\SFML-2.3.2
WXWIDGETS_PATH = C:\wxWidgets-3.0.2
LIBARCHIVE_PATH = C:\libarchive-3.1.2
WIN_ZLIB_PATH = C:\zlib128-dll

# Modify this if you built SFML somewhere else.
WIN_SFML_BUILD_PATH = ${SFML_PATH}\build
##############################################################################

# You shouldn't have to edit anything below this line. #######################

VERSION = 1.3.2

##############################################################################
# BerryBots GUI common source files
SOURCES =  bbguimain.cpp guiprinthandler.cpp guimanager.cpp newmatch.cpp
SOURCES += outputconsole.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp
SOURCES += circle2d.cpp line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp
SOURCES += bbengine.cpp bblua.cpp gfxeventhandler.cpp rectangle.cpp stage.cpp
SOURCES += packagedialog.cpp packageship.cpp packagestage.cpp dockitem.cpp
SOURCES += dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp
SOURCES += menubarmaker.cpp guigamerunner.cpp runnerdialog.cpp runnerform.cpp
SOURCES += bbrunner.cpp resultsdialog.cpp replaybuilder.cpp sysexec.cpp
SOURCES += stagepreview.cpp
##############################################################################


##############################################################################
# BerryBots CLI common source files
CLI_SOURCES =  bbsfmlmain.cpp bbutil.cpp gfxmanager.cpp filemanager.cpp
CLI_SOURCES += circle2d.cpp line2d.cpp point2d.cpp sensorhandler.cpp zone.cpp
CLI_SOURCES += bbengine.cpp bblua.cpp gfxeventhandler.cpp rectangle.cpp
CLI_SOURCES += stage.cpp cliprinthandler.cpp clipackagereporter.cpp dockitem.cpp
CLI_SOURCES += dockshape.cpp docktext.cpp dockfader.cpp zipper.cpp guizipper.cpp
CLI_SOURCES += bbrunner.cpp replaybuilder.cpp
##############################################################################


##############################################################################
# Sources and flags for building on Windows
WIN_EXTRA_SOURCES =  relativebasedir.cpp relativerespath.cpp bbres.o
WIN_EXTRA_SOURCES += .\luajit\src\lua51.dll ${WIN_ZLIB_PATH}\zlib1.dll
WIN_EXTRA_SOURCES += ${LIBARCHIVE_PATH}\build\libarchive\libarchive_static.a

WIN_CFLAGS =  -I.\luajit\src -I.\stlsoft-1.9.116\include
WIN_CFLAGS += -I${LIBARCHIVE_PATH}\libarchive -I${SFML_PATH}\include 
WIN_CFLAGS += -mthreads -DHAVE_W32API_H -D__WXMSW__ -DUNICODE
WIN_CFLAGS += -I${WXWIDGETS_PATH}\lib\gcc_lib\mswu -I${WXWIDGETS_PATH}\include
WIN_CFLAGS += -Wno-ctor-dtor-privacy -Wno-deprecated-declarations -pipe
WIN_CFLAGS += -fmessage-length=0 -std=gnu++11

WIN_LDFLAGS =  -L${WIN_SFML_BUILD_PATH}\lib -lpthread
WIN_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
WIN_LDFLAGS += -mwindows -mthreads -L${WXWIDGETS_PATH}\lib\gcc_lib
WIN_LDFLAGS += -lwxmsw30u_html -lwxmsw30u_adv -lwxmsw30u_core -lwxbase30u_xml
WIN_LDFLAGS += -lwxbase30u_net -lwxbase30u -lwxtiff -lwxjpeg -lwxpng -lwxzlib
WIN_LDFLAGS += -lwxregexu -lwxexpat -lkernel32 -luser32 -lgdi32 -lcomdlg32
WIN_LDFLAGS += -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32
WIN_LDFLAGS += -luuid -lrpcrt4 -ladvapi32 -lwsock32

WINCLI_EXTRA_SOURCES = relativebasedir.cpp relativerespath.cpp
WINCLI_EXTRA_SOURCES += .\luajit\src\lua51.dll ${WIN_ZLIB_PATH}\zlib1.dll
WINCLI_EXTRA_SOURCES += ${LIBARCHIVE_PATH}\build\libarchive\libarchive_static.a

WINCLI_CFLAGS =  -I.\luajit\src -I.\stlsoft-1.9.116\include
WINCLI_CFLAGS += -I${LIBARCHIVE_PATH}\libarchive -I${SFML_PATH}\include 
WINCLI_CFLAGS += -Wno-deprecated-declarations -std=gnu++11

WINCLI_LDFLAGS =  -L${WIN_SFML_BUILD_PATH}\lib -lpthread
WINCLI_LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
##############################################################################


##############################################################################
# Build targets
GUIPLATS = osx linux windows
CLIPLATS = rpi osxcli linuxcli wincli

none:
	@echo "Please do                            "
	@echo "  make TARGET                        "
	@echo "where TARGET is one of:              "
	@echo "  windows  wincli  winclean          "
	@echo "This Makefile is only here for Windows. On *nix, you should run:"
	@echo "  ./configure  # overwrites Makefile "
	@echo "  make                               "
	@echo "  make install # (optional)          "

MAKE_LUAJIT=cd luajit; $(MAKE); cd ..
CLEAN_LUAJIT=cd luajit; $(MAKE) clean; cd ..

WIN_MAKE_LUAJIT=cd luajit && $(MAKE) && cd ..
WIN_CLEAN_LUAJIT=cd luajit && $(MAKE) clean && cd ..

windows:
	windres --include-dir=${WXWIDGETS_PATH}\include berrybots.rc bbres.o
	$(WIN_MAKE_LUAJIT)	
	$(CC) ${SOURCES} ${WIN_EXTRA_SOURCES} ${WIN_CFLAGS} ${WIN_LDFLAGS} -o BerryBots
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-graphics-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-system-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-window-2.dll .
	copy ${WIN_ZLIB_PATH}\zlib1.dll .
	copy .\luajit\src\lua51.dll .
	@echo "==== Successfully built BerryBots $(VERSION) ===="
	@echo "==== Launch BerryBots with: BerryBots.exe"

wincli:
	$(WIN_MAKE_LUAJIT)	
	$(CC) ${CLI_SOURCES} ${WINCLI_EXTRA_SOURCES} ${WINCLI_CFLAGS} ${WINCLI_LDFLAGS} -o berrybots
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-graphics-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-system-2.dll .
	copy ${WIN_SFML_BUILD_PATH}\lib\sfml-window-2.dll .
	copy ${WIN_ZLIB_PATH}\zlib1.dll .
	copy .\luajit\src\lua51.dll .
	@echo "==== Successfully built BerryBots $(VERSION) ===="
	@echo "==== Launch BerryBots with: berrybots.exe"

winclean:
	$(WIN_CLEAN_LUAJIT)
	del *o
	del *.dll
	del BerryBots.exe
