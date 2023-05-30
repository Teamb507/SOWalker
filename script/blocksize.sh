#!/bin/bash

set -e

TW_DATASET="/dataset/twitter/twitter_rv.net"
CF_DATASET="/dataset/friendster/friendster.txt"
UK_DATASET="/dataset/uk-union/uk-union.txt"


for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 1342174728  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 536870912  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 1073741824  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 2147483648  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done
