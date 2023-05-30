
#!/bin/bash

# exit immediately if a command exists with non-zero exports
set -e

RANDOM=4521
dataset="$(basename $1)"
dirpath="$(dirname $1)"

echo "start to process dataset : $1"
awk -v seed=$RANDOM 'BEGIN{srand(seed)} {print $0"\t"int(rand() * 1000)}' $1 > ${dirpath}/w-${dataset}
echo "successfully done!"
