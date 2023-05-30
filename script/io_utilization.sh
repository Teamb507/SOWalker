
set -e

TW_DATASET="/dataset/twitter/twitter_rv.net"
CF_DATASET="/dataset/friendster/friendster.txt"
UK_DATASET="/dataset/uk-union/uk-union.txt"

for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule sa >>sowalker_log
done
sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET $CF_DATASET $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 268435456 cache_size 4 length 80 walkpersource 10 p 2.0 q 0.5 schedule gw >>graphwalker_log
done