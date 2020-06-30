#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage : $0 DETECTOR_NAME <number_of_cycles>"
    echo "           DETECTOR_NAME: JF07 or JF01..."
    echo "           number_of_cycles : optional, default 100"
    exit
fi

DETECTOR=$1
case ${DETECTOR} in
'JF01')
  D=1
  ;;
'JF02')
  D=2
  ;;
'JF06')
  D=6
  ;;
'JF07')
  D=7
  ;;
'JF13')
  D=13
  ;;
*)
  echo "Unsupported detector"
  exit
  ;;
esac

n_cycles=100
if [ $# == 2 ]
then
    n_cycles=$2
fi

export PATH=/home/dbe/miniconda3/bin:$PATH
source deactivate
source activate dia

sls_detector_put ${D}-timing trigger
sls_detector_put ${D}-cycles ${n_cycles}
sls_detector_put ${D}-exptime 5e-06
sls_detector_put ${D}-frames 1
sls_detector_put ${D}-dr 16
#sls_detector_put ${D}-clearbit to 0x5d 0 # normal mode, not highG0
sls_detector_put ${D}-status start

echo "Now start trigger"
