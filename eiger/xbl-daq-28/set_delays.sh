#!/bin/bash

NUM_MOD=20
# Unit are in 10ns
DELAY_DIFF=50000

for i in `seq 0 $((NUM_MOD-1))`; do
	sls_detector_put $i:txndelay_left  $((i*2*DELAY_DIFF))
	sls_detector_put $i:txndelay_right $(((i*2+1)*DELAY_DIFF))
done
    sls_detector_put txndelay_frame  $((NUM_MOD*2*DELAY_DIFF))
