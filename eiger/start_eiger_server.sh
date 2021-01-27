#!/bin/bash
# usage ./start_eiger_detector.sh Eiger 1
if [ $# -lt 1 ]
then
    echo "Usage : $0 <port>"
    echo "           DETECTOR_NAME: Eiger"
    echo "           tcp port : optional, default 2000"
    exit
fi

SLS_DET_PACKAGE_PATH='/home/hax_l/software/sf_daq_buffer/slsDetectorPackage/build/bin/'

port=2000
if [ $# == 2 ]
then
    port=$1
fi

echo "Starting the server..."
${SLS_DET_PACKAGE_PATH}eigerDetectorServerMaster_virtual -p port


