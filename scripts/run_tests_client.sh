#!/bin/bash

. ./set_env.sh
ulimit -n 8192

rm -rf logs
mkdir logs

declare -a test_scenarios=(20 100 200 500 1000 2000)
declare -a requests_types=(1 2 3 4 5 6 7)
# declare -a requests_types=(2 3 4 6)

IP=$(dig +short myip.opendns.com @resolver1.opendns.com)

log_file=""

for scenario in "${test_scenarios[@]}"
do
    for type in "${requests_types[@]}"
    do
        echo "Requests: $scenario type: $type"
        ./client -i $IP -a $master_ip -r $type -n $scenario | tee logs/log-client-$scenario-$type.txt
        sleep 1
    done
done
