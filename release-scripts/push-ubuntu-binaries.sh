#! /bin/bash -e

if [[ "$#" -ne 1 ]]; then
  echo "Please supply target, e.g. 'ubuntu1804'"
  exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="${1}"
package_dest_name_str="${TARGET}_VERSION"
package_dest_name="${!package_dest_name_str}"

BUILD_PATH=/app/build
PACKAGE="brewtarget_2.4.0_x86_64.deb"
IMG_NAME="cgspeck/brewtarget-build:${TARGET}-${TRAVIS_BUILD_NUMBER}"
echo "Pulling $IMG_NAME"
docker pull $IMG_NAME
echo -e "\n\nCopying ${BUILD_PATH}/${package_dest_name}"
docker run --rm --entrypoint cat $IMG_NAME $BUILD_PATH/$PACKAGE > "packages/${package_dest_name}"
echo -e "\n\nPackages:"
ls packages/

echo -e "\nDownloading github-releases tool"
tmp_dir=$(mktemp -d)
github_release_archive=$tmp_dir/github-release.bz2
github_release_path=$tmp_dir/github-release
wget "https://github.com/meterup/github-release/releases/download/v0.7.5/linux-amd64-github-release.bz2" -O $github_release_archive
bunzip2 $github_release_archive
chmod +x $github_release_path
echo -e "\nUploading binaries to Github"

src="./packages/${package_dest_name}"
echo -e "\nUploading ${src}"
$github_release_path upload \
  --user cgspeck \
  --repo brewtarget \
  --tag $TAG_NAME \
  --file $src \
  --name "${package_dest_name}"
