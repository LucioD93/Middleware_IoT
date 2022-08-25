#!/bin/bash

ulimit -n 8192

rm -rf logs
mkdir logs

./master -g 4 -w | tee logs/master.txt
