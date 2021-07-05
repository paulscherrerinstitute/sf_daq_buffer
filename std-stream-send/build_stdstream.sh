#!/bin/bash

VERSION=1.0.0

docker build --no-cache=true -f debug.Dockerfile -t paulscherrerinstitute/std-stream-send-sim .
docker tag paulscherrerinstitute/std-stream-send-sim paulscherrerinstitute/std-stream-send-sim:$VERSION

docker login
docker push paulscherrerinstitute/std-stream-send-sim:$VERSION
docker push paulscherrerinstitute/std-stream-send-sim