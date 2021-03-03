#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF11-buffer-worker@{00..03}
    exit
fi

M=$1

coreAssociatedBuffer=(11 12 13 1)

initialUDPport=50170
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF11T04V01

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
