#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF07-buffer-worker@{00..31}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

#8 udp2buffer workers per core
#coreAssociatedBuffer=(4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 8 8 8 8 8 8 8 8)
#4 udp2buffer workers per core
coreAssociatedBuffer=(4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8 9 9 9 9 10 10 10 10 11 11 11 11)
#2 udp2buffer workers per core
#coreAssociatedBuffer=(4 4 5 5 6 6 7 7 8 8 9 9 10 10 11 11 12 12 13 13 14 14 15 15 16 16 17 17 18 18 19 19)
# 1 udp2buffer worker per core
#coreAssociatedBuffer=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32)

initialUDPport=50100
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF07T32V01

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
