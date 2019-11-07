#! /bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

if [[ "$#" -eq 1 ]]; then
  TAG_NAME=$1
  echo "Overriding tag, using ${TAG_NAME}"
fi

BUILD_PATH=/app/build

if [[ "$TRAVIS" == "true" ]]; then
  echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
fi

echo "Building for ${TAG_NAME}"

tag="latest"

docker build \
  -t cgspeck/brewtarget-guacgui:$tag \
  -f Dockerfile-guacgui \
  --build-arg BUILD_DATE=$(date -u +%s) \
  --build-arg VERSION=$TAG_NAME \
  .

if [[ "$TRAVIS" == "true" ]]; then
  echo -e "\nPushing new docker images"
  docker push cgspeck/brewtarget-guacgui:$tag
  echo -e "\nUpdating Docker Hub Readme"
  docker run -v $PWD/docker/guacgui:/workspace \
    -e DOCKERHUB_USERNAME=$DOCKER_USERNAME \
    -e DOCKERHUB_PASSWORD=$DOCKER_PASSWORD \
    -e DOCKERHUB_REPOSITORY='cgspeck/brewtarget-guacgui' \
    -e README_FILEPATH='/workspace/README.md' \
    peterevans/dockerhub-description:2.1.0
fi
