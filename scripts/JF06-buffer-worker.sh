#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF06-buffer-worker@{00..31}
    exit
fi

M=$1

coreAssociatedBuffer=(4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8 9 9 9 9 10 10 10 10 11 11 11 11)

initialUDPport=50060
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF06T32V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
