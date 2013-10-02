#!/bin/bash

SFML_LIBNAMES="libsfml-graphics libsfml-system libsfml-window"

if [ ! -e ./sfml-lib ]
  then
  echo "Copying SFML libs..."
  mkdir sfml-lib
  cd sfml-lib
  for TARGET in ${SFML_LIBNAMES} ; do
    cp $1/${TARGET}.2.1.dylib .
    ln -s ${TARGET}.2.1.dylib ${TARGET}.2.dylib
    ln -s ${TARGET}.2.1.dylib ${TARGET}.dylib
  done
  cd ..

  for TARGET in ${SFML_LIBNAMES} ; do
    LIBFILE=sfml-lib/${TARGET}.2.1.dylib
    TARGETID=`otool -DX ${LIBFILE}`
    install_name_tool -id ${LIBFILE} ${LIBFILE}
    for TARGET2 in ${SFML_LIBNAMES} ; do
      LIBFILE2=sfml-lib/${TARGET2}.2.1.dylib
      install_name_tool -change ${TARGETID} ${LIBFILE} ${LIBFILE2}
    done
  done
  echo "  done!"
fi
