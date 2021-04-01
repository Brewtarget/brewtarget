#! /bin/bash -e
SRC_PATH=$1
PATCH="${SRC_PATH}\build-scripts\windows\patches\001-omit-missing-libs-for-packager.patch"

echo -e "\nApplying patches\n"

cd $SRC_PATH
git apply --stat $PATCH
git apply --check $PATCH
git apply $PATCH
cd -