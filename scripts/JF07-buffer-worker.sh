#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF07-buffer-worker@{00..31}
    exit
fi

M=$1

#8 udp2buffer workers per core
#coreAssociatedBuffer=(1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4)
#4 udp2buffer workers per core
#coreAssociatedBuffer=(1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8)
#2 udp2buffer workers per core
coreAssociatedBuffer=(1 1 2 2 3 3 4 4 5 5 6 6 7 7 8 8 9 9 10 10 11 11 12 12 13 13 14 14 15 15 16 16)

initialUDPport=50100
# strange that it doesn't work to add 08 or 09 (value too great for base (error token is "09"))
port=$((${initialUDPport}+10#${M}))
#port=`expr ${initialUDPport} + ${M}`
DETECTOR=JF07T32V01
echo ${port}

#taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} > /gpfs/photonics/swissfel/buffer/${port}.log
taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
