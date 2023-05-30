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
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done
for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 2147483648 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa 
done
sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 268435456 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule gw
done
for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 2147483648 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule gw 
done


sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = sa"
    ./bin/test/autoregressive $DATASET  c_update blocksize 268435456 cache_size 4 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule sa 
done

for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = sa"
    ./bin/test/autoregressive $DATASET  c_update blocksize 2147483648 cache_size 4 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule sa 
done

sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'

for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = gw"
    ./bin/test/autoregressive $DATASET  blocksize 268435456 cache_size 4 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule gw
done

for DATASET in $K30_DATASET $CW_DATASET
do
    echo "apps = autoregressive, dataset = $DATASET, length = 20, schedule = gw"
    ./bin/test/autoregressive $DATASET  blocksize 2147483648 cache_size 4 length 20 walkpersource 1 walksource 2000 alpha 0.2 schedule gw 
done


# DATASET=$SL_DATASET
# 
# case $1 in
#     "SL") DATASET=$SL_DATASET;;
#     "TW") DATASET=$TW_DATASET;;
#     "CF") DATASET=$CF_DATASET;;
#     "UK") DATASET=$UK_DATASET;;
#     "RM27") DATASET=$RM27_DATASET;;
#     "RM28") DATASET=$RM28_DATASET;;
#     *) DATASET=$SL_DATASET
# esac

# sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'

# for DATASET in $SL_DATASET $TW_DATASET $CF_DATASET $UK_DATASET $RM27_DATASET $RM28_DATASET
# do
#     echo "apps = autoregressive, dataset = $DATASET, length = 20, sample = reject"
#     ./bin/test/node2vec $DATASET sample reject length 20 walkpersource 1
# done

# for DATASET in $SL_DATASET $TW_DATASET $CF_DATASET $RM27_DATASET $RM28_DATASET
# do
#     echo "apps = autoregressive, dataset = $DATASET, length = 20, sample = reject"
#     ./bin/test/node2vec $DATASET sample reject length 20 walkpersource 1
# done

# for DATASET in $SL_DATASET $TW_DATASET $CF_DATASET $UK_DATASET $RM27_DATASET $RM28_DATASET
# do
#     echo "apps = autoregressive, dataset = $DATASET, length = 20, sample = reject"
#     ./bin/test/autoregressive $DATASET sample its length 20 walksource 1000000
# done

# for CACHE in 1 2 4 6 8 10
# do
#     echo "apps = node2vec, dataset = $CF_DATASET, length = 20, sample = reject, cache_size = $CACHE"
#     ./bin/test/node2vec $CF_DATASET sample reject length 20 walkpersource 1 cache $CACHE
# done

# for DATASET in $TW_DATASET $CF_DATASET $RM28_DATASET
# do
#     for ITER in 100 200 300 400 500 600 700 800 900 1000
#     do
#         echo "apps = node2vec, dataset = $DATASET, length = 20, sample = reject"
#         ./bin/test/node2vec $DATASET sample reject length 20 walkpersource 1 iter $ITER
#     done
# done

# for ITER in 400 800 900 1000
# do
#     echo "apps = node2vec, dataset = $UK_DATASET, length = 20, sample = reject"
#     ./bin/test/node2vec $UK_DATASET sample reject length 20 walkpersource 1 iter $ITER
# done

# for DATASET in $CF_DATASET $UK_DATASET $RM28_DATASET
# do
#     for ITER in 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000
#     do
#         echo "apps = node2vec, dataset = $DATASET, length = 20, sample = reject"
#         ./bin/test/node2vec $DATASET sample reject length 20 walkpersource 1 iter $ITER
#     done
# done

./bin/test/node2vec $TW_DATASET sample reject length 20 walkpersource 1
# ./bin/test/autoregressive $SL_DATASET sample its length 20 walksource 1000000