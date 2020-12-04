#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF06_4M-buffer-worker@{00..07}
    exit
fi

M=$1

coreAssociatedBuffer=(4 5 6 7 8 9 10 11)

initialUDPport=50060
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF06T08V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
