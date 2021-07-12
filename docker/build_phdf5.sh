#!/bin/bash

VERSION=1.0.1

docker build --no-cache=true -f phdf5.Dockerfile -t paulscherrerinstitute/sf-daq_phdf5 .
docker tag paulscherrerinstitute/sf-daq_phdf5 paulscherrerinstitute/sf-daq_phdf5:$VERSION

docker login
docker push paulscherrerinstitute/sf-daq_phdf5:$VERSION
docker push paulscherrerinstitute/sf-daq_phdf5