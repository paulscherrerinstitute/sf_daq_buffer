for i in {1..10}
do
   echo "Acquire $i"
   sls_detector_acquire
   stat=`sls_detector_get status`
   echo "$stat"
done
