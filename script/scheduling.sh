#!/bin/bash

set -e

UK_DATASET="/data/uk/uk-union.txt"

for scheduler in sa random greedy lp 
do
    echo "apps = node2vec, dataset = $UK_DATASET, length = 80"
    ./bin/test/node2vec $UK_DATASET  c_update blocksize 268435456 cache_size 4 skip length 80 walkpersource 10 p 2.0 q 0.5 schedule $scheduler
done
cp randgraph_metrics.txt /log/schedule_log
rm randgraph_metrics.txt
