#!/bin/bash

# path to build dir
BUILD_PATH='/home/dbe/git/sf_daq_buffer/build/'
# executables
UDP_RECV='std_udp_recv'
UDP_SYNC='std_udp_sync'
EIGER_ASSEMBLER='eiger_assembler'

# default config file
# CONFIG_FILE='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/eiger-5M.json'
CONFIG_FILE='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/eiger-1M.json'

HELP_FLAG=0
BIT_DEPTH=16
while getopts n:h:u:c:b: flag
do
    case "${flag}" in
        h ) HELP_FLAG=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
        b ) BIT_DEPTH=${OPTARG};;
    esac
done


N_UDP_RECVS="`cat ${CONFIG_FILE} | grep -P '"n_modules":' | grep -o '[0-9]\+'`"


# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -n <n_udp_recvs> -c <compile_flag> -h <help_flag> -u <udp_executable> -c <config_file> -b <bit_depth>"
    echo "           n_udp_recvs : number of receivers."
    echo "           help_flag : show this help and exits."
    echo "           config_file : detector configuration file."
    echo "           bit_depth : detector bit depth."
    exit
fi

# Start the receivers
echo "Starting ${N_UDP_RECVS} udp receivers..."
COUNTER=0
if [ -f "${UDP_RECV}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
         while [  $COUNTER -lt ${N_UDP_RECVS} ]; do
            "${BUILD_PATH}${UDP_RECV} ${CONFIG_FILE} ${COUNTER} ${BIT_DEPTH}" &
            let COUNTER=COUNTER+1 
            sleep 0.5
         done
    else
        echo "Something went wrong while starting the ${UDP_RECV}..."
        exit
    fi
else
    echo "Error: ${UDP_RECV} wasn't found..."
    exit
fi


# Start the std-udp-sync
echo "Starting the ${UDP_SYNC}..."
COUNTER=0
if [ -f "${UDP_SYNC}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        if [ ${BIT_DEPTH} -ne 0 ]; then
            "${BUILD_PATH}${UDP_SYNC} ${CONFIG_FILE} ${BIT_DEPTH}" &
            sleep 0.5
        else
            echo "Error: ${BIT_DEPTH} can't be zero..."
            exit
        fi
        
    else
        echo "Something went wrong while starting the ${UDP_SYNC}..."
        exit
    fi
else
    echo "Error: ${UDP_SYNC} wasn't found..."
    exit
fi


# Start the eiger assembler
echo "Starting the ${EIGER_ASSEMBLER}..."
COUNTER=0
if [ -f "${EIGER_ASSEMBLER}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        "${BUILD_PATH}${EIGER_ASSEMBLER} ${CONFIG_FILE} ${BIT_DEPTH}" &
        sleep 0.5
    else
        echo "Something went wrong while starting the ${EIGER_ASSEMBLER}..."
        exit
    fi
else
    echo "Error: ${EIGER_ASSEMBLER} wasn't found..."
    exit
fi