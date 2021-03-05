#!/bin/bash
N_UDP_RECVS=4
MAKE_FLAG=0
UDP_RECV='/home/hax_l/software/sf_daq_buffer/build/eiger_udp_recv'
CONFIG_FILE='/home/hax_l/software/sf_daq_buffer/eiger/config/eiger.json'
BUILD_PATH='/home/hax_l/software/sf_daq_buffer/build'
HELP_FLAG=0
while getopts p:n:m:h:u:c: flag
do
    case "${flag}" in
        p ) PORT=${OPTARG};;
        n ) N_UDP_RECVS=${OPTARG};;
        m ) MAKE_FLAG=${OPTARG};;
        h ) HELP_FLAG=${OPTARG};;
        u ) UDP_RECV=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
    esac
done

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -n <n_udp_recvs> -c <compile_flag> -h <help_flag> -u <udp_executable> -c <config_file>"
    echo "           n_udp_recvs : number of receivers."
    echo "           compile_flag : compile code before running."
    echo "           help_flag : show this help and exits."
    echo "           udp_executable : executable for the udp receivers."
    echo "           config_file : detector configuration file."
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
         while [  $COUNTER -lt 4 ]; do
            ${UDP_RECV} ${CONFIG_FILE} ${COUNTER} &
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
