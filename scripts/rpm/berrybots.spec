Name:           berrybots
Version:        1.3.3
Release:        1%{?dist}
Summary:        A cross-platform Lua programming game.

License:        zlib
URL:            http://berrybots.com
Source0:        berrybots-1.3.3.tar.gz

BuildRequires: atk-devel
BuildRequires: binutils
BuildRequires: cairo-devel
BuildRequires: ccache
BuildRequires: cmake
BuildRequires: dwz
BuildRequires: dyninst
BuildRequires: elfutils
BuildRequires: file
BuildRequires: freetype-devel
BuildRequires: gcc-c++
BuildRequires: gdb
BuildRequires: gdk-pixbuf2-devel
BuildRequires: glew-devel
BuildRequires: glib2-devel
BuildRequires: glibc
BuildRequires: glibc-common
BuildRequires: glibc-devel
BuildRequires: glibc-headers
BuildRequires: gtk2-devel
BuildRequires: hostname
BuildRequires: kernel
BuildRequires: kernel-headers
BuildRequires: libX11-devel
BuildRequires: libXrandr-devel
BuildRequires: libXrender-devel
BuildRequires: libXxf86vm-devel
BuildRequires: libarchive-devel
BuildRequires: libiscsi
BuildRequires: libjpeg-turbo-devel
BuildRequires: libpng-devel
BuildRequires: libsndfile-devel
BuildRequires: libsoup-devel
BuildRequires: libstdc++-devel
BuildRequires: llvm-libs
BuildRequires: mesa-libGL-devel
BuildRequires: mesa-libGLU-devel
BuildRequires: openal-soft-devel
BuildRequires: pango-devel
BuildRequires: pkgconfig
BuildRequires: tracker
BuildRequires: webkitgtk-devel
BuildRequires: xorg-x11-proto-devel
BuildRequires: xulrunner
BuildRequires: xz-devel
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
