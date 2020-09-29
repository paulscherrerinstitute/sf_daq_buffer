#!/bin/bash
echo $$ > /sys/fs/cgroup/cpuset/user/tasks
coreAssociated="33,34,35"

taskset -c ${coreAssociated} /usr/bin/sf_stream /gpfs/photonics/swissfel/buffer/config/stream-JF02.json
