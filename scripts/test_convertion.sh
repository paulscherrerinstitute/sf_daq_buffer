#!/bin/bash

export PATH=/home/dbe/miniconda3/bin:$PATH
source deactivate >/dev/null 2>&1
source activate conversion

#export NUMBA_NUM_THREADS=$1
#OUTDIR=/sf/alvra/data/p18674/raw/run_info/003000/CONVERSION-PAR-${NUMBA_NUM_THREADS}
OUTDIR=/sf/alvra/data/p18674/raw/run_info/003000/CONVERSION-NEW.NO-LOAD.3-PIN

#coreAssociatedBuffer=(35 34 33 32 31 30 29 28 27 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 8 7 6 5 4 3 2 1 0)
#coreAssociatedBuffer=(35 34 33 32 31 30 29 28 27 18 19 20 21 22 23 24 25 26 9 10 11 12 13 14 15 16 17 8 7 6 5 4 3 2 1 0)

coreAssociatedBuffer=(35 26 34 25 33 24 32 23 31 22 30 21 29 20 28 19 27 18 17 8 16 7 15 6 14 5 13 4 12 3 11 2 10 1 9)

coreAssociated="35,26,34,25,33,24,32,23,31,22,30,21,29,20,28,19,27,18"

#for N in 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31 33 35
for N in 1 12 14 16 18 20 2 4 6 8 10 22 24 26 28 30 32 35
do

    for n in `seq -f %02g 1 $N`
    do
        rm -rf /sf/alvra/data/p18674/raw/run_info/003000/conversion_0030${n}.log
        sleep 0.1

        c=`echo $n - 1 | bc`
        echo process : $n cores : ${coreAssociatedBuffer[10#${c}]}
        taskset -c ${coreAssociatedBuffer[10#${c}]} python /home/dbe/git/sf_daq_buffer/scripts/export_file.py /sf/alvra/data/p18674/raw//RAW_DATA/test_16M/run_0030${n}.JF06T32V02.h5 /sf/alvra/data/p18674/raw/test_16M/run_0030${n}.JF06T32V02.h5 /sf/alvra/data/p18674/raw/run_info/003000/run_0030${n}.json /gpfs/photonics/swissfel/buffer/config/stream-JF06.json > /sf/alvra/data/p18674/raw/run_info/003000/conversion_0030${n}.log &

#        echo process : $n cores :${coreAssociated}
#        rm -rf /sf/alvra/data/p18674/raw/run_info/003000/conversion_0030${n}.log
#        taskset -c ${coreAssociated}               python /home/dbe/git/sf_daq_buffer/scripts/export_file.py /sf/alvra/data/p18674/raw//RAW_DATA/test_16M/run_0030${n}.JF06T32V02.h5 /sf/alvra/data/p18674/raw/test_16M/run_0030${n}.JF06T32V02.h5 /sf/alvra/data/p18674/raw/run_info/003000/run_0030${n}.json /gpfs/photonics/swissfel/buffer/config/stream-JF06.json > /sf/alvra/data/p18674/raw/run_info/003000/conversion_0030${n}.log &

    done
    echo Submitted

    A=0
    while [ $A -lt 30 ]
    do
        sleep 30
        A=`grep read /sf/alvra/data/p18674/raw/run_info/003000/conversion_003001.log | wc -l`
        echo Number of cycles passed $A
    done

    K=`ps -fe | grep export | grep -v grep | awk '{print $2}' | xargs`
    echo Killing `ps -fe | grep export | grep -v grep | awk '{print $2}' | wc -l` processes ${K}
    kill -9 ${K}

    sleep 2

    mkdir -p ${OUTDIR}/${N}
    mv /sf/alvra/data/p18674/raw/run_info/003000/conversion_0030* ${OUTDIR}/${N}/.

done
