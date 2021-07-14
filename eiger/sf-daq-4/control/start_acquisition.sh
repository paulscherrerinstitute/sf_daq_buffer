#!/bin/bash
ENDPOINT="http://127.0.0.1:5000"
SYNC="/write_sync"
ASYNC="/write_async"
KILL="/kill"
HEADER="Content-Type:application/json"
N_IMAGES=5
SOURCES="eiger"
# Define a timestamp function
timestamp() {
  date +"%T" # current time
}
generate_post_data()
{
  cat <<EOF
{"n_images": 5, "output_file": "$OUTPUT_FILE", "sources": "eiger"}
EOF
}

for((i=1;i<=10;i+=1)); do 
    TIMESTAMP=$(date +%d-%m-%Y_%H-%M-%S)
    echo "[${TIMESTAMP}] Starting Request ${i}..."
    OUTPUT_FILE="/home/hax_l/tests/output_"${i}"_"${TIMESTAMP}".h5"
    # echo "$(generate_post_data)"
    curl -X POST -H ${HEADER} --data "$(generate_post_data)" ${ENDPOINT}${SYNC}
    
    
done