#! /bin/bash -ex
SRC_PATH=/app/brewtarget
BUILD_PATH=/app/build

echo -e "\nApplying patches\n"
patch=/app/brewtarget/docker/ubuntu1804/patches/0001-Revert-CMakelist-change-because-2.3.4-is-not-availab.patch
cd $SRC_PATH
git apply --stat $patch
git apply --check $patch
# git am < $patch  # <- am would do an apply & commit
git apply $patch
echo -e "\nStarting build...\n"
[ -d $BUILD_PATH ] && rm -rf $BUILD_PATH
mkdir $BUILD_PATH
cd $BUILD_PATH
cmake $SRC_PATH
make
make package
