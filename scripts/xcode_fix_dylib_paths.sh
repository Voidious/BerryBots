#!/bin/bash
# Thanks ynniv http://ynniv.com/blog/2006/02/deploying-app-that-use-dylibs-on-mac.html

EXECFILE=${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}
LIBPATH=${BUILT_PRODUCTS_DIR}/${SHARED_SUPPORT_FOLDER_PATH}
NEWLIBPATH="@executable_path/../SharedSupport"

# space separated list of libraries
TARGETS="libsfml-network.2.0.dylib libsfml-audio.2.0.dylib libsfml-system.2.0.dylib libsfml-graphics.2.0.dylib libsfml-window.2.0.dylib libwx_baseu-2.9.4.0.0.dylib libwx_baseu_net-2.9.4.0.0.dylib libwx_baseu_xml-2.9.4.0.0.dylib libwx_osx_cocoau_adv-2.9.4.0.0.dylib libwx_osx_cocoau_aui-2.9.4.0.0.dylib libwx_osx_cocoau_core-2.9.4.0.0.dylib libwx_osx_cocoau_gl-2.9.4.0.0.dylib libwx_osx_cocoau_html-2.9.4.0.0.dylib libwx_osx_cocoau_media-2.9.4.0.0.dylib libwx_osx_cocoau_propgrid-2.9.4.0.0.dylib libwx_osx_cocoau_qa-2.9.4.0.0.dylib libwx_osx_cocoau_ribbon-2.9.4.0.0.dylib libwx_osx_cocoau_richtext-2.9.4.0.0.dylib libwx_osx_cocoau_stc-2.9.4.0.0.dylib libwx_osx_cocoau_webview-2.9.4.0.0.dylib libwx_osx_cocoau_xrc-2.9.4.0.0.dylib"
for TARGET in ${TARGETS} ; do
  LIBFILE=${LIBPATH}/${TARGET}
  TARGETID=`otool -DX ${LIBPATH}/$TARGET`
  NEWTARGETID=${NEWLIBPATH}/${TARGET}
  install_name_tool -id ${NEWTARGETID} ${LIBFILE}
  install_name_tool -change ${TARGETID} ${NEWTARGETID} ${EXECFILE}
  for TARGET2 in ${TARGETS} ; do
    LIBFILE2=${LIBPATH}/${TARGET2}
    install_name_tool -change ${TARGETID} ${NEWTARGETID} ${LIBFILE2}
  done
done

# TODO: create symlinks here so we can use this copy for the library search path
