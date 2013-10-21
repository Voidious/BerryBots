AC_INIT([BerryBots], [1.3.0], [voidious@berrybots.com], [berrybots], [http://berrybots.com])
m4_include(scripts/wxwin.m4)
AC_PROG_CXX
AC_LANG([C++])

AC_MSG_NOTICE([Checking host OS...])
AC_CONFIG_AUX_DIR([scripts])
AC_CANONICAL_HOST

case "${host_os}" in
  linux*)  target=linux;;
  darwin*) target=osx;;
  *)       target=unknown;;
esac

case "${host_cpu}" in
  armv6*) rpi=1
          target=rpi
          ;;
esac

if test "$target" == unknown; then
  AC_MSG_WARN([
    Couldn't figure out build target for:
      host_cpu = ${host_cpu}
      host_vendor = ${host_vendor}
      host_os = ${host_os}
    Setting to 'none'. You'll need to specify a build target for 'make'.])
else
  AC_MSG_NOTICE([
    Build target set to: ${target}
      host_cpu = ${host_cpu}
      host_os = ${host_os}
      host_vendor = ${host_vendor}])
fi


# Raspberry Pi doesn't depend on anything.
if test "$rpi" != 1; then
  withSfml=1
  withWx=1
  withLa=1
  AC_ARG_WITH([sfml], [(optional) Path to SFML, e.g., /home/voidious/SFML-2.1],
              [AC_SUBST([sfml], $withval)], [withSfml=0])
  AC_ARG_WITH([wx], [(optional) Path to wxWidgets, e.g., /home/voidious/wxWidgets-2.9.5],
              [AC_SUBST([wx], $withval)], [withWx=0])
  AC_ARG_WITH([libarchive], [(optional) Path to libarchive, e.g., /home/voidious/libarchive-3.1.2],
              [AC_SUBST([libarchive], $withval)], [withLa=0])

  AC_MSG_NOTICE([
    Checking for wxWidgets...])
  AM_OPTIONS_WXCONFIG
  reqwx=2.9.0
  AM_PATH_WXCONFIG($reqwx, wxFound=1)

  AS_IF([test "$withWx" == 1], [AC_MSG_NOTICE([
    Using provided wxWidgets install.])],
        [test "$wxFound" == 1], [AC_MSG_NOTICE([
    Found wxWidgets installed on system.])],
        [AC_MSG_ERROR([
    wxWidgets not found and not provided. You must either install wxWidgets
    or provide a path to a wxWidgets that you downloaded or compiled.

    Example install: "sudo apt-get install libwxgtk2.9-dev"
    Example provide: "./configure --with-wx=/path/to/wx"

    If you think wxWidgets is installed, please check that wx-config is in the
    path, the directory where wxWidgets libraries are installed (returned by
    'wx-config --libs' or 'wx-config --static --libs' command) is in
    LD_LIBRARY_PATH or equivalent variable and wxWidgets version is $reqwx
    or above.
  ])])


  AC_MSG_NOTICE([
    Checking for SFML...])
  sfmlFound=1
  AC_CHECK_LIB([sfml-system], [main], [], [sfmlFound=0])
  AC_CHECK_LIB([sfml-window], [main], [], [sfmlFound=0])
  AC_CHECK_LIB([sfml-graphics], [main], [], [sfmlFound=0])
  AC_CHECK_HEADER([SFML/System.hpp], [], [sfmlFound=0])
  AC_CHECK_HEADER([SFML/Graphics.hpp], [], [sfmlFound=0])
  AC_CHECK_HEADER([SFML/System/Vector2.hpp], [], [sfmlFound=0])
  AC_CHECK_HEADER([SFML/Graphics/Rect.hpp], [], [sfmlFound=0])
  AC_CHECK_HEADER([SFML/Graphics/RectangleShape.hpp], [], [sfmlFound=0])

  AS_IF([test "$withSfml" == 1], [AC_MSG_NOTICE([
    Using provided SFML install.])],
        [test "$sfmlFound" == 1], [AC_MSG_NOTICE([
    Found SFML installed on system.])],
        
        [AC_MSG_ERROR([
    SFML not found and not provided. You must either install SFML or provide
    a path to an SFML that you downloaded or compiled.

    Example install: "sudo apt-get install libsfml-dev"
    Example provide: "./configure --with-sfml=/path/to/sfml"
  ])])


  AC_MSG_NOTICE([
    Checking for libarchive...])
  laFound=1
  AC_CHECK_LIB([archive], [main], [], [laFound=0])
  AC_CHECK_HEADER([archive.h], [], [laFound=0])
  AC_CHECK_HEADER([archive_entry.h], [], [laFound=0])

  AS_IF([test "$withLa" == 1], [AC_MSG_NOTICE([
    Using provided libarchive install.])],
        [test "$laFound" == 1], [AC_MSG_NOTICE([
    Found libarchive installed on system.])],
        [AC_MSG_ERROR([
    libarchive not found and not provided. You must either install libarchive
    or provide a path to a libarchive that you downloaded or compiled.

    Example install: "sudo apt-get install libarchive-dev"
    Example provide: "./configure --with-libarchive=/path/to/libarchive"
  ])])


  AC_CHECK_LIB([z], [main], [], [AC_MSG_ERROR([
    zlib not found on your system.
  ])])
fi


AC_SUBST([target], $target)
AC_OUTPUT(Makefile)
