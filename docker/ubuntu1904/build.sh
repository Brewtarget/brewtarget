#! /bin/bash -ex
SRC_PATH=/app/brewtarget
BUILD_PATH=/app/build
[ -d $BUILD_PATH ] && rm -rf $BUILD_PATH
mkdir $BUILD_PATH
cd $BUILD_PATH
cmake $SRC_PATH
make
make package
