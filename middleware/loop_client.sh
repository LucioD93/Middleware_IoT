#!/bin/bash

# This sets the output of `time` to only the real time in milliseconds
TIMEFORMAT=%R

NUMBER_OF_REQUESTS=$1

function execute() {
  START=$(date +%s%N)
  ./client -r $1 > /dev/null
  END=$(date +%s%N)
  echo "Request: $1 - Elapsed time: $(($((END-START))/1000000)) ms"
}

# Create array of random numbers between 1 and 6
entries=($(shuf -i 1-6 -n $NUMBER_OF_REQUESTS -r))

GLOBAL_START=$(date +%s%N)

for entry in "${entries[@]}"; do
  execute $entry
done

GLOBAL_END=$(date +%s%N)

echo "Total elapsed time: $(($((GLOBAL_END-GLOBAL_START))/1000000))ms"
echo "Average time per request: $(($(($((GLOBAL_END-GLOBAL_START))/1000000))/$((NUMBER_OF_REQUESTS))))ms"
