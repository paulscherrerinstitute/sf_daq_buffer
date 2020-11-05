#!/bin/bash

coreAssociated="2,3,4,5"
#echo $$ > /sys/fs/cgroup/cpuset/user/tasks

taskset -c ${coreAssociated} /usr/local/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF06.daq8.json
