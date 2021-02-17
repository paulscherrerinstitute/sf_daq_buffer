#!/bin/bash

if [ -z ${SLS_DET_PACKAGE_PATH} ]; then SLS_DET_PACKAGE_PATH="/home/hax_l/software/sf_daq_buffer/slsDetectorPackage/build/bin/"; fi

${SLS_DET_PACKAGE_PATH}sls_detector_put triggers 1
${SLS_DET_PACKAGE_PATH}sls_detector_put timing trigger
${SLS_DET_PACKAGE_PATH}sls_detector_put exptime 0.000005
${SLS_DET_PACKAGE_PATH}sls_detector_put frames 1
${SLS_DET_PACKAGE_PATH}sls_detector_put dr 16
${SLS_DET_PACKAGE_PATH}sls_detector_put start

echo "Acquisition started..."
