#!/bin/bash

# conda activate
export PATH=/home/dbe/miniconda2/bin:$PATH
source /home/dbe/miniconda2/etc/profile.d/conda.sh
conda deactivate
conda activate sf-daq

# path to build dir
BUILD_PATH='/home/dbe/git/sf_daq_buffer/build/'
# executables
UDP_RECV='std_udp_recv'
UDP_SYNC='std_udp_sync'
EIGER_ASSEMBLER='eiger_assembler'
STD_DET_WRITER='std_det_writer'

# default config file
CONFIG_FILE='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/eiger.json'
DET_CONFIG_FILE='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/5m.config'
DET_PARAM_FILE='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/test.par'

HELP_FLAG=0

# CONFIGURATION
BIT_DEPTH=16
N_MPI_EXEC=3
while getopts h:c:b:m: flag
do
    case "${flag}" in
        h ) HELP_FLAG=${OPTARG};;
        c ) CONFIG_FILE=${OPTARG};;
        b ) BIT_DEPTH=${OPTARG};;
        m ) N_MPIT_EXEC=${OPTARG};;
    esac
done

# prints help and exits
if (( ${HELP_FLAG} == 1 )); then
    echo "Usage : $0 -h <help_flag> -c <config_file> -b <bit_depth>"
    echo "           help_flag : show this help and exits."
    echo "           config_file : detector configuration file."
    echo "           bit_depth : detector bit depth."
    exit
fi

if [ -f "${CONFIG_FILE}" ]; then
    N_UDP_RECVS="`cat ${CONFIG_FILE} | grep -P '"n_modules":' | grep -o '[0-9]\+'`"
else
    echo "Something went wrong with the config file... ${CONFIG_FILE}..."
    exit
fi

if [ ${N_MPI_EXEC} -gt 0 ]
then
    echo "Number of mpi writers: ${N_MPI_EXEC}."
else
    echo "Something went wrong with the number of mpi writer processes..."
    exit
fi



if [ -f "${CONFIG_FILE}" ]; then
    N_UDP_RECVS="`cat ${CONFIG_FILE} | grep -P '"n_modules":' | grep -o '[0-9]\+'`"
else
    echo "Something went wrong with the config file... ${CONFIG_FILE}..."
    exit
fi

# Start the receivers
echo "Starting ${N_UDP_RECVS} udp receivers..."
COUNTER=0
if [ -f "${BUILD_PATH}${UDP_RECV}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        if [ ${BIT_DEPTH} -ne 0 ]; then
            while [  $COUNTER -lt ${N_UDP_RECVS} ]; do
                ${BUILD_PATH}${UDP_RECV} ${CONFIG_FILE} ${COUNTER} ${BIT_DEPTH} &
                let COUNTER=COUNTER+1 
                sleep 0.5
            done
        else
            echo "Error: ${BIT_DEPTH} can't be zero..."
            exit
        fi
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
if [ -f "${BUILD_PATH}${UDP_SYNC}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        ${BUILD_PATH}${UDP_SYNC} ${CONFIG_FILE} &
        sleep 0.5
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
if [ -f "${BUILD_PATH}${EIGER_ASSEMBLER}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        if [ ${BIT_DEPTH} -ne 0 ]; then
            ${BUILD_PATH}${EIGER_ASSEMBLER} ${CONFIG_FILE} ${BIT_DEPTH} &
            sleep 0.5
        else
            echo "Error: ${BIT_DEPTH} can't be zero..."
            exit
        fi
    else
        echo "Something went wrong while starting the ${EIGER_ASSEMBLER}..."
        exit
    fi
else
    echo "Error: ${EIGER_ASSEMBLER} wasn't found..."
    exit
fi

# Start the eiger writer
echo "Starting the ${STD_DET_WRITER}..."
export PATH="/usr/lib64/mpich-3.2/bin:${PATH}";
export LD_LIBRARY_PATH="/usr/lib64/mpich-3.2/lib:${LD_LIBRARY_PATH}";

if [ -f "${BUILD_PATH}${STD_DET_WRITER}" ]; then
    if [ -f "${CONFIG_FILE}" ]; then
        mpiexec -n ${N_MPI_EXEC} ${BUILD_PATH}${STD_DET_WRITER} ${CONFIG_FILE} ${BIT_DEPTH} &
        sleep 0.5
    else
        echo "Something went wrong while starting the ${STD_DET_WRITER}..."
        exit
    fi
else
    echo "Error: ${STD_DET_WRITER} wasn't found..."
    exit
fi


# prepares the detector
echo "Preparing the detector configuration for a default acquisition...";
export PATH="/home/dbe/git/sf_daq_buffer/slsDetectorPackage/build/bin/:${PATH}";
sls_detector_put config ${DET_CONFIG_FILE}
sls_detector_put parameters ${DET_PARAM_FILE}
sls_detector_put trimbits /home/dbe/git/sf_daq_buffer/eiger/test/eiger.sn000
sls_detector_put dac vthreshold 2050
