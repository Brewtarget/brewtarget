#! /bin/bash -e

if [[ "$#" -ne 1 ]]; then
  echo "Please supply target, e.g. 'ubuntu1804' or 'ubuntu1904'"
  exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="${1}"
package_dest_name_str="${TARGET}_VERSION"
package_dest_name="${!package_dest_name_str}"

BUILD_PATH=/app/build
PACKAGE="brewtarget_2.4.0_x86_64.deb"


echo -e "\n\nCopying ${BUILD_PATH}/${package_dest_name}"
docker run --rm --entrypoint cat cgspeck/brewtarget-build:$TARGET $BUILD_PATH/$PACKAGE > "packages/${package_dest_name}"
echo -e "\n\nPackages:"
ls packages/

echo -e "\nDownloading github-releases tool"
  tmp_dir=$(mktemp -d)
  github_release_archive=$tmp_dir/github-release.tar.bz2
  github_release_path=$tmp_dir/bin/linux/amd64/github-release
  wget "https://github.com/aktau/github-release/releases/download/v0.7.2/linux-amd64-github-release.tar.bz2" -O $github_release_archive
  tar xvjf $github_release_archive -C $tmp_dir
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
