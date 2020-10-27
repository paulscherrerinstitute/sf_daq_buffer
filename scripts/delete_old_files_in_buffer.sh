#!/bin/bash

hours=5
threshold=80

if [ $# -ge 1 ]
then
    threshold=$1
fi
if [ $# -eq 2 ]
then
    hours=$2
fi

df -h | grep BUFFER > /dev/null
if [ $? != 0 ]
then
    # BUFFER is not present 
    exit
fi

occupancy=`df -h /gpfs/photonics/swissfel/buffer | grep BUFFER | awk '{print $5}' | sed 's/%//'`
if [ ${occupancy} -lt ${threshold} ]
then
#    echo OK, not action
    exit
fi

find /gpfs/photonics/swissfel/buffer/JF* -type f -mmin +$((${hours}*60)) -delete
find /gpfs/photonics/swissfel/buffer/JF*/M* -type d -mmin +$((${hours}*60)) -delete
