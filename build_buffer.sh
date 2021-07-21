#!/bin/bash

VERSION=1.0.0

docker build --no-cache=true -t paulscherrerinstitute/std-daq-buffer .
docker tag paulscherrerinstitute/std-daq-buffer paulscherrerinstitute/std-daq-buffer:$VERSION

docker login
docker push paulscherrerinstitute/std-daq-buffer:$VERSION
docker push paulscherrerinstitute/std-daq-buffer