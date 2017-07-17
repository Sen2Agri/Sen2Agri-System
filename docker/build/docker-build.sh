#!/bin/sh
docker build \
    -t sen2agri-build \
    --build-arg user=$USER \
    --build-arg group=$(id -ng) \
    --build-arg uid=$(id -u) \
    --build-arg gid=$(id -g) \
    .
