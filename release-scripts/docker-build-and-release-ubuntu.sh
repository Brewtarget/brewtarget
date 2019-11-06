#! /bin/bash -e

if [[ "$#" -ne 1 ]]; then
  echo "Please supply target, e.g. 'ubuntu1804' or 'ubuntu1904'"
  exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="${1}"
BUILD_PATH=/app/build
PACKAGES=("brewtarget_2.4.0_x86_64.deb" "brewtarget_2.4.0_x86_64.rpm" "brewtarget_2.4.0_x86_64.tar.bz2")

if [[ "$TRAVIS" == "true" ]]; then
  echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
fi

echo "Building for ${TARGET}"

tag="${TARGET}"
expanded_tag="${tag}-${SHORT_HASH}"

docker build -t cgspeck/brewtarget-build:$tag -f Dockerfile-$TARGET .
for package in ${PACKAGES[*]}; do
  docker run --rm --entrypoint cat cgspeck/brewtarget-build:$tag $BUILD_PATH/$package > "packages/${TARGET}_${package}"
done
echo -e "\nPackages:"
ls packages/
echo -e "\nPushing new docker images"
docker push cgspeck/brewtarget-build:$tag

echo -e "\nDownloading github-releases tool"
tmp_dir=$(mktemp -d)
github_release_archive=$tmp_dir/github-release.tar.bz2
github_release_path=$tmp_dir/bin/linux/amd64/github-release
wget "https://github.com/aktau/github-release/releases/download/v0.7.2/linux-amd64-github-release.tar.bz2" -O $github_release_archive
tar xvjf $github_release_archive -C $tmp_dir
chmod +x $github_release_path
echo -e "\nUploading binaries to Github"


for package in ${PACKAGES[*]}; do
  src="./packages/${TARGET}_${package}"
  echo -e "\nUploading ${src}"
  $github_release_path upload \
    --user cgspeck \
    --repo brewtarget \
    --tag $TAG_NAME \
    --file $src \
    --name "${TARGET}_${package}"
done
