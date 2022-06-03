#!/bin/bash

for _ in {1..10000}; do
  ./client -r $(( $RANDOM % 6 + 1 ));
  sleep 0.05
done
