#!/bin/sh
docker build \
    -t sen2agri-build \
    --build-arg user=$USER \
    --build-arg group=$(id -ng) \
    --build-arg uid=$(id -u) \
    --build-arg gid=$(id -g) \
    .

docker run -it --rm \
       -v $(realpath ../..):/sen2agri \
       -u $(id -u):$(id -g) \
       sen2agri-build /bin/bash entry.sh
