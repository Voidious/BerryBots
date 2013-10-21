#!/bin/bash

SFML_LIBNAMES="libsfml-graphics libsfml-system libsfml-window"
SFML_VERSION="2.0"

rm -rf ./sfml-lib
if [ ! -e ./sfml-lib ]
  then
  echo "Copying SFML libs..."
  mkdir sfml-lib
  cd sfml-lib
  for TARGET in ${SFML_LIBNAMES} ; do
    cp $1/${TARGET}.${SFML_VERSION}.dylib .
    ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.2.dylib
    ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.dylib
  done
  cd ..

  for TARGET in ${SFML_LIBNAMES} ; do
    LIBFILE=sfml-lib/${TARGET}.${SFML_VERSION}.dylib
    TARGETID=`otool -DX ${LIBFILE}`
    install_name_tool -id ${LIBFILE} ${LIBFILE}
    for TARGET2 in ${SFML_LIBNAMES} ; do
      LIBFILE2=sfml-lib/${TARGET2}.${SFML_VERSION}.dylib
      install_name_tool -change ${TARGETID} ${LIBFILE} ${LIBFILE2}
    done
  done
  echo "  done!"
fi
