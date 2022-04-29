#! /bin/bash
while : ; do sleep 1 && echo $(date) && top -b -p 12790,12775,12774,12778 -n 1 | tail -n4;  
#while true; do:
#    sleep 1
#    echo $(date)
#    top -b -p 12790,12775,12774, 12778 -n1 | tail -4
#    if [[ $input = "q" ]] || [[ $input = "Q" ]] 
#    then
#        echo "Q pressed, quitting..."
#        break
#    fi;
done
