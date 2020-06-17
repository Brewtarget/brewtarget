#! /bin/bash -ex
SRC_PATH=/app/brewtarget
BUILD_PATH=/app/build

echo -e "\nApplying patches\n"
patch=/app/brewtarget/docker/ubuntu2004/patches/0001-Update-CMakelist-dependencies.patch
cd $SRC_PATH
git apply --stat $patch
git apply --check $patch
git apply $patch

echo -e "\nStarting build...\n"
[ -d $BUILD_PATH ] && rm -rf $BUILD_PATH
mkdir $BUILD_PATH
cd $BUILD_PATH
cmake $SRC_PATH
make
# QT_QPA_PLATFORM=minimal make test
make package