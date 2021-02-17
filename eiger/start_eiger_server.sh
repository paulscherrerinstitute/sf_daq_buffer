#!/bin/bash
# usage ./start_eiger_detector.sh Eiger 1
if [ $# -lt 1 ]
then
    echo "Usage : $0 <port>"
    echo "           tcp port : optional, default 2000"
    exit
fi

SLS_DET_PACKAGE_PATH='/home/hax_l/software/sf_daq_buffer/slsDetectorPackage/build/bin/'
PORT=2050
if [ $# == 2 ]
then
    PORT=$1
fi

echo "Starting the server..."

echo ${SLS_DET_PACKAGE_PATH}eigerDetectorServerMaster_virtual -p ${PORT}


