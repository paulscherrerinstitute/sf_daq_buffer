#!/bin/bash

VERSION=1.0.2

docker build --no-cache=true -f buffer-base.Dockerfile -t paulscherrerinstitute/sf-daq-buffer-base .
docker tag paulscherrerinstitute/sf-daq-buffer-base paulscherrerinstitute/sf-daq-buffer-base:$VERSION

docker login
docker push paulscherrerinstitute/sf-daq-buffer-base:$VERSION
docker push paulscherrerinstitute/sf-daq-buffer-base