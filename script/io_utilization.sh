
set -e

TW_DATASET="/dataset/twitter/twitter_rv.net"
CF_DATASET="/dataset/friendster/friendster.txt"
UK_DATASET="/dataset/uk-union/uk-union.txt"

for DATASET in $TW_DATASET 
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa >>log/tw_sowalker_log
done
for DATASET in $CF_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa >>log/fr_sowalker_log
done
for DATASET in $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = sa"
    ./bin/test/node2vec $DATASET  c_update blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule sa >>log/uk_sowalker_log
done
sudo sync; sudo sh -c '/usr/bin/echo 1 > /proc/sys/vm/drop_caches'
for DATASET in $TW_DATASET 
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule gw >>log/tw_graphwalker_log
done
for DATASET in $CF_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule gw >>log/fr_graphwalker_log
done
for DATASET in $UK_DATASET
do
    echo "apps = node2vec, dataset = $DATASET, length = 80, schedule = gw"
    ./bin/test/node2vec $DATASET  blocksize 268435456  length 80 walkpersource 10 p 2.0 q 0.5 schedule gw >>log/uk_graphwalker_log
done
python3 ./plot/fig_7a.py
python3 ./plot/fig_7b.py