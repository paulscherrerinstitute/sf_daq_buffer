#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF10-buffer-worker@00
    exit
fi

M=$1

coreAssociatedBuffer=(35)

initialUDPport=50160
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF10T01V01

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
