#!/bin/bash

for _ in {1..1000}; do
  ./client -r $(( $RANDOM % 6 + 1 ));
  sleep 0.01
done
