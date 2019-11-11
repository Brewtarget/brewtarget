#! /bin/bash -e
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="arch"
BUILD_PATH=/app/arch
PACKAGE="brewtarget-devel-${ARCH_VERSION}-1-x86_64.pkg.tar.xz"
IMG_NAME="cgspeck/brewtarget-build:${TARGET}-${TRAVIS_BUILD_NUMBER}"

docker pull $IMG_NAME
docker run \
    --rm \
    --entrypoint cat $IMG_NAME $BUILD_PATH/$PACKAGE > "packages/${TARGET}_${PACKAGE}"

echo -e "\nPackages:"
ls packages/

echo -e "\nDownloading github-releases tool"
tmp_dir=$(mktemp -d)
github_release_archive=$tmp_dir/github-release.tar.bz2
github_release_path=$tmp_dir/bin/linux/amd64/github-release
wget "https://github.com/aktau/github-release/releases/download/v0.7.2/linux-amd64-github-release.tar.bz2" -O $github_release_archive
tar xvjf $github_release_archive -C $tmp_dir
chmod +x $github_release_path
echo -e "\nUploading binaries to Github"


src="./packages/${TARGET}_${PACKAGE}"
echo -e "\nUploading ${src}"
$github_release_path upload \
    --user cgspeck \
    --repo brewtarget \
    --tag $TAG_NAME \
    --file $src \
    --name "${TARGET}_${PACKAGE}"
