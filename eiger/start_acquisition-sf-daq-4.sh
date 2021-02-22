#!/bin/bash
# usage ./start_eiger_detector.sh Eiger 1
if [ $# -lt 1 ]
then
    echo "Usage : $0 DETECTOR_NAME <number_of_cycles>"
    echo "           DETECTOR_NAME: Eiger..."
    echo "           number_of_cycles : optional, default 100"
    exit
fi

#SLS_DET_PACKAGE_PATH='/home/dbe/git/sf_daq_buffer_eiger/slsDetectorPackage/build/bin/'
SLS_DET_PACKAGE_PATH=''
DETECTOR=$1

n_cycles=1
if [ $# == 2 ]
then
    n_cycles=$2
fi


${SLS_DET_PACKAGE_PATH}sls_detector_put timing trigger
${SLS_DET_PACKAGE_PATH}sls_detector_put triggers ${n_cycles}
${SLS_DET_PACKAGE_PATH}sls_detector_put exptime 0.000005
${SLS_DET_PACKAGE_PATH}sls_detector_put frames 10
${SLS_DET_PACKAGE_PATH}sls_detector_put dr 16
#sls_detector_put ${D}-clearbit to 0x5d 0 # normal mode, not highG0
${SLS_DET_PACKAGE_PATH}sls_detector_put start

echo "Now start trigger"