#!/bin/bash
N_UDP_RECVS=2
MAKE_FLAG=0
UDP_RECV='/home/hax_l/sf_daq_buffer/build/eiger_udp_recv'
CONFIG_FILE='/home/hax_l/sf_daq_buffer/eiger/sf-daq-4/config/eiger.json'
BUILD_PATH='/home/hax_l/sf_daq_buffer/build'
HELP_FLAG=0
BIT_DEPTH=16
while getopts n:m:h:u:c:b: flag
do
    case "${flag}" in
        n ) N_UDP_RECVS=${OPTARG};;
        m ) MAKE_FLAG=${OPTARG};;
        h ) HELP_FLAG=${OPTARG};;
        u ) UDP_RECV=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
        b ) BIT_DEPTH=${OPTARG};;
    esac
done

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -n <n_udp_recvs> -c <compile_flag> -h <help_flag> -u <udp_executable> -c <config_file> -b <bit_depth>"
    echo "           n_udp_recvs : number of receivers."
    echo "           compile_flag : compile code before running."
    echo "           help_flag : show this help and exits."
    echo "           udp_executable : executable for the udp receivers."
    echo "           config_file : detector configuration file."
    echo "           bit_depth : detector bit depth."
    exit
fi

# compiles the sf_buffer_daq 
if (( ${MAKE_FLAG} == 1 )); then
    echo "Compile flag detected..."
    set -e
    cd ${BUILD_PATH} && make
fi

# proceeds to start the receivers
echo "Starting ${N_UDP_RECVS} udp receivers..."
COUNTER=0
if [ -f "${UDP_RECV}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
         while [  $COUNTER -lt ${N_UDP_RECVS} ]; do
            ${UDP_RECV} ${CONFIG_FILE} ${COUNTER} ${BIT_DEPTH} &
            let COUNTER=COUNTER+1 
            sleep 0.5
         done
    else
        echo "Something went wrong with the udp recv executable..."
        exit
    fi
else
    echo "Something went wrong with the udp recv executable..."
    exit
fi
