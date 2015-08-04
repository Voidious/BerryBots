#!/bin/bash

SFML_LIBNAMES="libsfml-graphics libsfml-system libsfml-window"
SFML_VERSION="2.3"

if [ -e $2 ]
  then
  for TARGET in ${SFML_LIBNAMES} ; do
    LIBFILE=$2/${TARGET}.${SFML_VERSION}.dylib
    TARGETID=`otool -DX ${LIBFILE}`
    install_name_tool -id ${LIBFILE} ${LIBFILE}
    for TARGET2 in ${SFML_LIBNAMES} ; do
      LIBFILE2=$2/${TARGET2}.${SFML_VERSION}.dylib
      install_name_tool -change ${TARGETID} ${LIBFILE} ${LIBFILE2}
    done
    install_name_tool -change ${TARGETID} ${LIBFILE} $1
  done
  echo "  done!"
fi
