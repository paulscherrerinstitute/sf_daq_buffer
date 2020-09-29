#!/bin/bash

if [ $# != 1 ]
then
    systemctl start JF02-buffer-worker@{00..08}
    exit
fi

M=$1

# Add ourselves to the user cpuset.
echo $$ > /sys/fs/cgroup/cpuset/user/tasks

#coreAssociatedBuffer=(25 25 26 26 27 27 28 28 29)
coreAssociatedBuffer=(1 2 2 3 3 4 4 5 5)

initialUDPport=50020
port=$((${initialUDPport}+10#${M}))
DETECTOR=JF02T09V02

taskset -c ${coreAssociatedBuffer[10#${M}]} /usr/bin/sf_buffer ${DETECTOR} M${M} ${port} /gpfs/photonics/swissfel/buffer/${DETECTOR} ${M}
