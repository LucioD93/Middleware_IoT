#!/bin/bash

. ./set_env.sh
ulimit -n 8192

rm -rf logs
mkdir logs

./worker -a $sanfrancisco_master_ip -g 1 | tee logs/worker.txt
