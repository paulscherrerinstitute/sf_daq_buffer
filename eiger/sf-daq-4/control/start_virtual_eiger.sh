#!/bin/bash

MAKE_FLAG=0
MASTER='/home/hax_l/sf_daq_buffer/slsDetectorPackage/build/bin/eigerDetectorServerMaster_virtual'
SLAVE='/home/hax_l/sf_daq_buffer/slsDetectorPackage/build/bin/eigerDetectorServerSlaveBottom_virtual'
DET_PUT='/home/hax_l/sf_daq_buffer/slsDetectorPackage/build/bin/sls_detector_put'
CONFIG_FILE='/home/hax_l/sf_daq_buffer/eiger/sf-daq-4/config/veiger-sf-daq-4.txt'

HELP_FLAG=0
PORT_MASTER=2070
while getopts h:p:c: flag
do
    case "${flag}" in
        h ) HELP_FLAG=${OPTARG};;
        p ) PORT_MASTER=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
    esac
done

PORT_SLAVE=$(( $PORT_MASTER + 2 ))


# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -c <config_file> -h <help_flag>"
    echo "           port : port connection for master virtual eiger."
    echo "           config_file : detector configuration file."
    echo "           help_flag : show this help and exits."
    exit
fi


# proceeds to start the receivers
echo "Starting the virtual eiger (master and slave) (config file: ${CONFIG_FILE})"
if [ -f "${MASTER}" ]; then
    if [ -f "${MASTER}" ]; then
        if [ -f "${CONFIG_FILE}" ]; then
            ${MASTER} -p ${PORT_MASTER} &
            sleep 1
            ${SLAVE} -p ${PORT_SLAVE} &
            sleep 1
            # ${DET_PUT} config ${CONFIG_FILE}  
        else
            echo "Something went wrong with the config file..."
            exit
        fi
    fi
else
    echo "Something went wrong with the virtual eiger executable..."
    exit
fi




