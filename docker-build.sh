#! /bin/bash -e
BUILD_PATH=/app/build
TARGETS=("ubuntu1804" "ubuntu1904")
PACKAGES=("brewtarget_2.4.0_x86_64.deb" "brewtarget_2.4.0_x86_64.rpm" "brewtarget_2.4.0_x86_64.tar.bz2")

for target in ${TARGETS[*]}; do
    tag="bt-${target}"
    docker build -t $tag -f Dockerfile-$target .
    for package in ${PACKAGES[*]}; do
        docker run --rm --entrypoint cat $tag $BUILD_PATH/$package > packages/$target/$package
    done
done
