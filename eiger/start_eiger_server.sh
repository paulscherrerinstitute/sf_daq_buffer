#!/bin/bash

# default port 2050 and 2052
if [ $# -eq 0 ]
then
    PORT=2050
    PORT_SLAVE=2052
elif [ $# -eq 1 ]
then
    PORT=$1
    PORT_SLAVE=$(( $1 + 2 ))
else
    echo "Usage : $0 <port>"
    echo "           tcp port : optional, default 2050"
    exit
fi

SLS_DET_PACKAGE_PATH='/home/hax_l/software/sf_daq_buffer/slsDetectorPackage/build/bin/'


echo "Starting the master eiger virtual detector at port ${PORT}..."

if [ -f "${SLS_DET_PACKAGE_PATH}eigerDetectorServerMaster_virtual" ]; then
    ${SLS_DET_PACKAGE_PATH}eigerDetectorServerMaster_virtual -p ${PORT} &
else
    echo "Eiger master was not found at ${SLS_DET_PACKAGE_PATH}."
    exit
fi

echo "Starting the slave bottom eiger virtual detector at port ${PORT_SLAVE}..."

if [ -f "${SLS_DET_PACKAGE_PATH}eigerDetectorServerSlaveBottom_virtual" ]; then
    ${SLS_DET_PACKAGE_PATH}eigerDetectorServerSlaveBottom_virtual -p ${PORT} &
else
    echo "Eiger slave bottom was not found at ${SLS_DET_PACKAGE_PATH}."
    exit
fi


