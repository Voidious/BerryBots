#!/bin/bash
# This script is used for compiling BerryBots on Mac OS X with XCode. For
# instructions, see:
# http://berrybots.com/wiki/Compiling_BerryBots_app_on_Mac_OS_X_with_XCode
#
# Thanks ynniv for this article:
# http://ynniv.com/blog/2006/02/deploying-app-that-use-dylibs-on-mac.html

EXECFILE=${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}
LIBPATH=${BUILT_PRODUCTS_DIR}/${SHARED_SUPPORT_FOLDER_PATH}

EXEC_FRAMEWORK_PATH="@executable_path/../Frameworks"
EXEC_SHARED_LIBPATH="@executable_path/../SharedSupport"
SFML_LIBNAMES="libsfml-system libsfml-graphics libsfml-window libsfml-audio libsfml-network"
SFML_VERSION="2.0"
WX_LIBNAMES="libwx_baseu-2.9 libwx_baseu_net-2.9 libwx_baseu_xml-2.9 libwx_osx_cocoau_adv-2.9 libwx_osx_cocoau_aui-2.9 libwx_osx_cocoau_core-2.9 libwx_osx_cocoau_gl-2.9 libwx_osx_cocoau_html-2.9 libwx_osx_cocoau_media-2.9 libwx_osx_cocoau_propgrid-2.9 libwx_osx_cocoau_qa-2.9 libwx_osx_cocoau_ribbon-2.9 libwx_osx_cocoau_richtext-2.9 libwx_osx_cocoau_stc-2.9 libwx_osx_cocoau_xrc-2.9"

for TARGET in ${SFML_LIBNAMES} ; do
  cd ${LIBPATH}
  if [ -e ${TARGET}.2.dylib ]; then
    rm ${TARGET}.2.dylib
  fi
  if [ -e ${TARGET}.dylib ]; then
    rm ${TARGET}.dylib
  fi
  ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.2.dylib
  ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.dylib
  LIBFILE=${LIBPATH}/${TARGET}.${SFML_VERSION}.dylib
  NEWTARGETID=${EXEC_SHARED_LIBPATH}/${TARGET}.${SFML_VERSION}.dylib
  install_name_tool -id ${NEWTARGETID} ${LIBFILE}
  for TARGET2 in ${SFML_LIBNAMES} ; do
    LIBFILE2=${LIBPATH}/${TARGET2}.${SFML_VERSION}.dylib
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.${SFML_VERSION}.dylib ${NEWTARGETID} ${LIBFILE2}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.2.dylib ${NEWTARGETID} ${LIBFILE2}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.dylib ${NEWTARGETID} ${LIBFILE2}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.${SFML_VERSION}.dylib ${NEWTARGETID} ${EXECFILE}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.2.dylib ${NEWTARGETID} ${EXECFILE}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.dylib ${NEWTARGETID} ${EXECFILE}
  done
done

for TARGET in ${WX_LIBNAMES} ; do
  cd ${LIBPATH}
  if [ -e ${TARGET}.5.dylib ]; then
    rm ${TARGET}.5.dylib
  fi
  if [ -e ${TARGET}.dylib ]; then
    rm ${TARGET}.dylib
  fi
  ln -s ${TARGET}.5.0.0.dylib ${TARGET}.5.dylib
  ln -s ${TARGET}.5.0.0.dylib ${TARGET}.dylib
  LIBFILE=${LIBPATH}/${TARGET}.5.0.0.dylib
  TARGETID=`otool -DX ${LIBFILE}`
  NEWTARGETID=${EXEC_SHARED_LIBPATH}/${TARGET}.5.0.0.dylib
  install_name_tool -id ${NEWTARGETID} ${LIBFILE}
  install_name_tool -change ${TARGETID} ${NEWTARGETID} ${EXECFILE}
  for TARGET2 in ${WX_LIBNAMES} ; do
    LIBFILE2=${LIBPATH}/${TARGET2}.5.0.0.dylib
    install_name_tool -change ${TARGETID} ${NEWTARGETID} ${LIBFILE2}
  done
done
