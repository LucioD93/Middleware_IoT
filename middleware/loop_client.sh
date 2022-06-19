#!/bin/bash

# This sets the output of `time` to only the real time in milliseconds
TIMEFORMAT=%R

NUMBER_OF_REQUESTS=$1
let DONE_REQUESTS=0

function execute() {
  START=$(date +%s%N)
  ./client -a 127.0.0.1 -r $1 > /dev/null
  END=$(date +%s%N)
  echo "Request: $1 - Elapsed time: $(($((END-START))/1000000)) ms"
  let DONE_REQUESTS=$((DONE_REQUESTS + 1))
}

# Create array of random numbers between 1 and 6
entries=($(shuf -i 1-6 -n $NUMBER_OF_REQUESTS -r))

GLOBAL_START=$(date +%s%N)

for entry in "${entries[@]}"; do
  execute $entry &
done

while true; do
  if [ "$DONE_REQUESTS" -eq "$NUMBER_OF_REQUESTS" ]; then
      break;
  fi
  sleep 1
done

GLOBAL_END=$(date +%s%N)

echo "Total elapsed time: $(($((GLOBAL_END-GLOBAL_START))/1000000))ms"
echo "Average time per request: $(($(($((GLOBAL_END-GLOBAL_START))/1000000))/$((NUMBER_OF_REQUESTS))))ms"

echo "$((DONE_REQUESTS))"
