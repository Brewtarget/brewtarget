#! /bin/bash -e

if [[ "$#" -ne 1 ]]; then
  echo "Please supply target, e.g. 'ubuntu1804', 'ubuntu1904' or 'ubuntu2004'"
  exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

TARGET="${1}"

if [[ "$TRAVIS" == "true" ]]; then
  echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
fi

echo "Building for ${TARGET}"

docker build \
  -t cgspeck/brewtarget-build:$TARGET \
  -f Dockerfile-$TARGET \
  .

IMG_NAME="cgspeck/brewtarget-build:${TARGET}-${TRAVIS_BUILD_NUMBER}"
echo "Creating Docker Tag: ${IMG_NAME}"
docker tag cgspeck/brewtarget-build:$TARGET $IMG_NAME

if [[ "$TRAVIS" == "true" && "$BRANCH_NAME" == "develop" ]]; then
  echo -e "\nPushing new docker image: cgspeck/brewtarget-build:$TARGET"
  docker push cgspeck/brewtarget-build:$TARGET
  echo -e "\nPushing new docker image: $IMG_NAME"
  docker push $IMG_NAME
fi
