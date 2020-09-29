#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF06-buffer-worker@{00..31}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

#coreAssociatedBuffer=(22 22 23 23 24 24 25 25 26 26 27 27 28 28 29 29 30 30 31 31 32 32 33 33 34 34 35 35 36 36 37 37)
coreAssociatedBuffer=(6 6 7 7 8 8 9 9 10 10 22 22 23 23 24 24 25 25 26 26 27 27 28 28 29 29 30 30 31 31 32 32)

initialUDPport=50060
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF06T32V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
