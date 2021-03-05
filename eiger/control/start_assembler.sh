#!/bin/bash

MAKE_FLAG=0
ASSEMBLER='/home/hax_l/software/sf_daq_buffer/build/eiger_assembler'
CONFIG_FILE='/home/hax_l/software/sf_daq_buffer/eiger/config/eiger.json'
BUILD_PATH='/home/hax_l/software/sf_daq_buffer/build'
NAME='eiger_assembler'
HELP_FLAG=0
while getopts m:h:c: flag
do
    case "${flag}" in
        m ) MAKE_FLAG=${OPTARG};;
        h ) HELP_FLAG=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
    esac
done

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -c <config_file> -m <make_flag> -h <help_flag>"
    echo "           config_file : detector configuration file."
    echo "           make_flag : compile code before running."
    echo "           help_flag : show this help and exits."
    exit
fi

# compiles the sf_buffer_daq 
if (( ${MAKE_FLAG} == 1 )); then
    echo "Compile flag detected..."
    set -e
    cd ${BUILD_PATH} && make
fi

# proceeds to start the receivers
echo "Starting the ${NAME} (config file: ${CONFIG_FILE})"
if [ -f "${ASSEMBLER}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        ${ASSEMBLER} ${CONFIG_FILE} &
    else
        echo "Something went wrong with the config file..."
        exit
    fi
else
    echo "Something went wrong with the assembler executable..."
    exit
fi
