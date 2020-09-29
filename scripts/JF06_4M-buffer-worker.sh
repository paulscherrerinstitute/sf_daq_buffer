#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF06_4M-buffer-worker@{00..07}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
# echo $$ > /sys/fs/cgroup/cpuset/user/tasks

coreAssociatedBuffer=(22 23 24 25 26 27 28 29)

initialUDPport=50060
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF06T08V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
