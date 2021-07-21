#!/bin/bash

VERSION=1.0.4

docker build --no-cache=true -f buffer-base.Dockerfile -t paulscherrerinstitute/std-daq-buffer-base .
docker tag paulscherrerinstitute/std-daq-buffer-base paulscherrerinstitute/std-daq-buffer-base:$VERSION

docker login
docker push paulscherrerinstitute/std-daq-buffer-base:$VERSION
docker push paulscherrerinstitute/std-daq-buffer-base