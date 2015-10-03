#!/bin/bash
# This script is used for compiling BerryBots on Mac OS X with XCode. For
# instructions, see:
# http://berrybots.com/wiki/Compiling_BerryBots_app_on_Mac_OS_X_with_XCode
#
# Thanks ynniv for this article:
# http://ynniv.com/blog/2006/02/deploying-app-that-use-dylibs-on-mac.html

EXECFILE=${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}
LIBPATH=${BUILT_PRODUCTS_DIR}/${SHARED_SUPPORT_FOLDER_PATH}

EXEC_FRAMEWORK_PATH="@rpath"
EXEC_SHARED_LIBPATH="@executable_path/../SharedSupport"
SFML_LIBNAMES="libsfml-system libsfml-graphics libsfml-window libsfml-audio libsfml-network"
SFML_VERSION="2.3.2"
SFML_MAJOR="2.3"
WX_VERSION="0.2.0"
WX_MAJOR="0"
WX_LIBNAMES="libwx_baseu-3.0 libwx_baseu_net-3.0 libwx_baseu_xml-3.0 libwx_osx_cocoau_adv-3.0 libwx_osx_cocoau_aui-3.0 libwx_osx_cocoau_core-3.0 libwx_osx_cocoau_gl-3.0 libwx_osx_cocoau_html-3.0 libwx_osx_cocoau_media-3.0 libwx_osx_cocoau_propgrid-3.0 libwx_osx_cocoau_qa-3.0 libwx_osx_cocoau_ribbon-3.0 libwx_osx_cocoau_richtext-3.0 libwx_osx_cocoau_stc-3.0 libwx_osx_cocoau_webview-3.0 libwx_osx_cocoau_xrc-3.0"

for TARGET in ${SFML_LIBNAMES} ; do
  cd ${LIBPATH}
  if [ -e ${TARGET}.${SFML_MAJOR}.dylib ]; then
    rm ${TARGET}.${SFML_MAJOR}.dylib
  fi
  if [ -e ${TARGET}.dylib ]; then
    rm ${TARGET}.dylib
  fi
  ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.${SFML_MAJOR}.dylib
  ln -s ${TARGET}.${SFML_VERSION}.dylib ${TARGET}.dylib
  LIBFILE=${LIBPATH}/${TARGET}.${SFML_VERSION}.dylib
  NEWTARGETID=${EXEC_SHARED_LIBPATH}/${TARGET}.${SFML_VERSION}.dylib
  install_name_tool -id ${NEWTARGETID} ${LIBFILE}
  for TARGET2 in ${SFML_LIBNAMES} ; do
    LIBFILE2=${LIBPATH}/${TARGET2}.${SFML_VERSION}.dylib
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.${SFML_VERSION}.dylib ${NEWTARGETID} ${LIBFILE2}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.${SFML_MAJOR}.dylib ${NEWTARGETID} ${LIBFILE2}
    install_name_tool -change ${EXEC_FRAMEWORK_PATH}/${TARGET}.dylib ${NEWTARGETID} ${LIBFILE2}
  done
done

for TARGET in ${WX_LIBNAMES} ; do
  cd ${LIBPATH}
  if [ -e ${TARGET}.${WX_MAJOR}.dylib ]; then
    rm ${TARGET}.${WX_MAJOR}.dylib
  fi
  if [ -e ${TARGET}.dylib ]; then
    rm ${TARGET}.dylib
  fi
  ln -s ${TARGET}.${WX_VERSION}.dylib ${TARGET}.${WX_MAJOR}.dylib
  ln -s ${TARGET}.${WX_VERSION}.dylib ${TARGET}.dylib
  LIBFILE=${LIBPATH}/${TARGET}.${WX_VERSION}.dylib
  TARGETID=`otool -DX ${LIBFILE}`
  NEWTARGETID=${EXEC_SHARED_LIBPATH}/${TARGET}.${WX_VERSION}.dylib
  install_name_tool -id ${NEWTARGETID} ${LIBFILE}
  for TARGET2 in ${WX_LIBNAMES} ; do
    LIBFILE2=${LIBPATH}/${TARGET2}.${WX_VERSION}.dylib
    install_name_tool -change ${TARGETID} ${NEWTARGETID} ${LIBFILE2}
  done
done
