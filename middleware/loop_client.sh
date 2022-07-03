#!/bin/bash

# This sets the output of `time` to only the real time in milliseconds
TIMEFORMAT=%R

NUMBER_OF_REQUESTS=$3
REQUEST_TYPE=$2
MASTER_IP=$1
let DONE_REQUESTS=0

function execute() {
  START=$(date +%s%N)
  ./client -a $MASTER_IP -r $1 > /dev/null
  END=$(date +%s%N)
  echo "Request: $1 - Elapsed time: $(($((END-START))/1000000)) ms"
  let DONE_REQUESTS=$((DONE_REQUESTS + 1))
}

# Create array of random numbers between 1 and 6
entries=($(shuf -i 1-6 -n $NUMBER_OF_REQUESTS -r))

GLOBAL_START=$(date +%s%N)

if [ $REQUEST_TYPE == "7" ]; then
   for entry in "${entries[@]}"; do
     execute "$entry"
   done

else
  for ((i=1;i<=NUMBER_OF_REQUESTS;i++)); do
      execute "$REQUEST_TYPE" &
  done
fi

while true; do
  if [ $DONE_REQUESTS -eq $NUMBER_OF_REQUESTS ]; then
      break;
  fi
  echo "DONE $DONE_REQUESTS"
  sleep 1
done

GLOBAL_END=$(date +%s%N)

echo "Total elapsed time: $(($((GLOBAL_END-GLOBAL_START))/1000000))ms"
echo "Average time per request: $(($(($((GLOBAL_END-GLOBAL_START))/1000000))/$((NUMBER_OF_REQUESTS))))ms"

echo "$((DONE_REQUESTS))"
