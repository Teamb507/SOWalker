#!/bin/bash

set -e

TW_DATASET="/dataset/twitter/twitter_rv.net"
CF_DATASET="/dataset/friendster/friendster.txt"
UK_DATASET="/dataset/uk-union/uk-union.txt"
K30_DATASET="/dataset/kron30/kron30.txt"
CW_DATASET="/dataset/crawl/crawl-web.txt"


for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 1073741824 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done
for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 1073741824 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done
cp randgraph_metrics.txt /log/sowalker_node2vec_log
rm randgraph_metrics.txt

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 1073741824 length 80 walkpersource 10 p 2.0 q 0.5 schedule gw
done
for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 1073741824  length 80 walkpersource 10 p 2.0 q 0.5 schedule gw 
done
cp randgraph_metrics.txt /log/graphwalker_node2vec_log
rm randgraph_metrics.txt


sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = sa"
    ./bin/test/autoregressive $DATASET  c_update blocksize 1073741824 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule sa 
done

for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = sa"
    ./bin/test/autoregressive $DATASET  c_update blocksize 1073741824 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule sa 
done
cp randgraph_metrics.txt /log/sowalker_pagerank_log
rm randgraph_metrics.txt

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'

for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = gw"
    ./bin/test/autoregressive $DATASET  blocksize 1073741824 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule gw
done

for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = gw"
    ./bin/test/autoregressive $DATASET  blocksize 1073741824 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule gw 
done
cp randgraph_metrics.txt /log/graphwalker_pagerank_log
rm randgraph_metrics.txt
python3 ./plot/fig_5a.py
python3 ./plot/fig_5b.py
python3 ./plot/fig_6a.py
python3 ./plot/fig_6b.py