#! /bin/bash -e
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source $DIR/common-vars

BUILD_PATH=/app/build

if [[ "$TRAVIS" == "true" ]]; then
  echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
fi

echo "Building for ${TAG_NAME}"

docker_tag="latest"

docker build \
  -t cgspeck/brewtarget-guacgui:$docker_tag \
  -f Dockerfile-guacgui \
  --build-arg BUILD_DATE=$(date -u +%s) \
  --build-arg VERSION=$TAG_NAME \
  --build-arg UBUNTU1804_DEB=$ubuntu1804_VERSION \
  .

if [[ "$TRAVIS" == "true" ]]; then
  echo -e "\nPushing new docker images"
  docker push cgspeck/brewtarget-guacgui:$docker_tag
fi
