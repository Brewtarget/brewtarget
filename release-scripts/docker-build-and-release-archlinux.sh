#! /bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="arch"
BUILD_PATH=/app/arch
PACKAGE="brewtarget-devel-${ARCH_VERSION}-1-x86_64.pkg.tar.xz"
BUILD_DATE="dev"

if [[ "$TRAVIS" == "true" ]]; then
  BUILD_DATE=$(date -u +%s)
  echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
fi

echo "Building for ${TARGET}"

docker build \
  -t cgspeck/brewtarget-build:$TARGET \
  -f Dockerfile-$TARGET \
  --build-arg BUILD_DATE \
  --build-arg VERSION=$ARCH_VERSION \
  .

IMG_NAME="cgspeck/brewtarget-build:${TARGET}-${TRAVIS_BUILD_NUMBER}"
echo "Creating Docker Tag: ${IMG_NAME}"
docker tag cgspeck/brewtarget-build:$TARGET $IMG_NAME

if [[ "$TRAVIS" == "true" && "$BRANCH" == "develop" ]]; then
  echo -e "\nPushing new docker image: cgspeck/brewtarget-build:$TARGET"
  docker push cgspeck/brewtarget-build:$TARGET
  echo -e "\nPushing new docker image: $IMG_NAME"
  docker push $IMG_NAME
fi
