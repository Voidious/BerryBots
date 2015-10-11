Name:           berrybots
Version:        1.3.3
Release:        1%{?dist}
Summary:        A cross-platform Lua programming game.

License:        zlib
URL:            http://berrybots.com
Source0:        berrybots-1.3.3.tar.gz

BuildRequires: atk
BuildRequires: atk-devel
BuildRequires: bash
BuildRequires: bind99-libs
BuildRequires: binutils
BuildRequires: cairo
BuildRequires: cairo-devel
BuildRequires: cmake
BuildRequires: coreutils
BuildRequires: cpio
BuildRequires: diffutils
BuildRequires: dwz
BuildRequires: dyninst
BuildRequires: elfutils
BuildRequires: expat
BuildRequires: expat-devel
BuildRequires: file
BuildRequires: findutils
BuildRequires: flac-devel
BuildRequires: flac-libs
BuildRequires: fontconfig
BuildRequires: freetype
BuildRequires: freetype-devel
BuildRequires: gawk
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: gdb
BuildRequires: gdk-pixbuf2
BuildRequires: gdk-pixbuf2-devel
BuildRequires: glib2
BuildRequires: glib2-devel
BuildRequires: glibc
BuildRequires: glibc-common
BuildRequires: glibc-devel
BuildRequires: glibc-headers
BuildRequires: grep
BuildRequires: gtk2
BuildRequires: gtk2-devel
BuildRequires: guile
BuildRequires: gzip
BuildRequires: hostname
BuildRequires: kernel-core
BuildRequires: kernel-headers
BuildRequires: libX11
BuildRequires: libX11-devel
BuildRequires: libXext
BuildRequires: libXxf86vm
BuildRequires: libXxf86vm-devel
BuildRequires: libgcc
BuildRequires: libiscsi
BuildRequires: libjpeg-turbo
BuildRequires: libjpeg-turbo-devel
BuildRequires: libogg
BuildRequires: libogg-devel
BuildRequires: libpng
BuildRequires: libpng-devel
BuildRequires: libstdc++
BuildRequires: libstdc++-devel
BuildRequires: libvorbis
BuildRequires: libvorbis-devel
BuildRequires: libxcb
BuildRequires: libxcb-devel
BuildRequires: llvm-libs
BuildRequires: make
BuildRequires: mesa-libGL
BuildRequires: mesa-libGL-devel
BuildRequires: mesa-libGLU
BuildRequires: mesa-libGLU-devel
BuildRequires: openal-soft
BuildRequires: openal-soft-devel
BuildRequires: pango
BuildRequires: pango-devel
BuildRequires: pkgconfig
BuildRequires: sed
BuildRequires: systemd-devel
BuildRequires: systemd-libs
BuildRequires: tar
BuildRequires: xcb-util-image
BuildRequires: xcb-util-image-devel
BuildRequires: xorg-x11-proto-devel
BuildRequires: xz
BuildRequires: zlib
BuildRequires: zlib-devel

%description
Spaceships, lasers, mazes, races, battles, and programmable stages! A fun and flexible Lua programming game for Mac, Linux, Windows, and Raspberry Pi.

%prep
%setup -q


%build
%{!?_with_localsfml: %{!?_without_localsfml: %define _with_localsfml --with-localsfml}}
%{!?_with_localwx: %{!?_without_localwx: %define _with_localwx --with-localwx}}
%{!?_with_localla: %{!?_without_localla: %define _with_localla --with-localla}}
%configure \
        %{?_with_localsfml} \
        %{?_with_localwx} \
        %{?_with_localla}
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%files
%doc LICENSE README
%{_bindir}/berrybots
%{_libdir}/berrybots/*
%{_datadir}/berrybots/*
%{_datadir}/applications/berrybots.desktop
%{_datadir}/pixmaps/berrybots.png


%changelog
* Thu Oct 10 2015 Patrick Cupka <pcupka@gmail.com> - 1.3.3-1
- BerryBots v1.3.3
* Thu Jul 07 2013 Patrick Cupka <pcupka@gmail.com> - 1.3.2-1
- Initial version of the package
