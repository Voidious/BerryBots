#!/bin/sh
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./sfml-lib
./bbmain $*

