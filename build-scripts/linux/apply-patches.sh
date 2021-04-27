#! /bin/bash -e
SRC_PATH=$1
PATCH="${SRC_PATH}/build-scripts/linux/patches/001-auto-discover-install-dependencies.patch"

echo -e "\nApplying patches\n"

cd $SRC_PATH
git apply --stat $PATCH
git apply --check $PATCH
git apply $PATCH
cd -