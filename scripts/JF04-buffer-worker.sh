#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF04-buffer-worker@00
    exit
fi

M=$1

coreAssociatedBuffer=(12)

initialUDPport=50040
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF04T01V01

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/local/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
