#!/bin/bash

F=$1
S=$2

t=`stat -c %y $F`

while true
do
    sleep 5
    t1=`stat -c %y $F`

    if [ "${t1}" != "${t}" ]
    then
        echo $F changed
        t=${t1} 
        systemctl restart ${S}
    fi

done

