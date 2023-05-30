#!/bin/bash

# exit immediately if a command exists with non-zero exports
set -e

# the download dataset name
dataset=uk-union.txt

# the dataset download directory
data_dir=$1

if [[ -z ${data_dir}  ]]
then
    data_dir=$(pwd)
fi

mkdir -p ${data_dir}


dataset_path=${data_dir}/${dataset}

echo "the ${dataset} dataset output path : ${dataset_path}"

if [[ ! -f ${dataset_path} ]] 
then      
    echo "start to download ${dataset} dataset, please wait..."
    wget http://data.law.di.unimi.it/webdata/uk-union-2006-06-2007-05/uk-union-2006-06-2007-05-underlying.graph -O ${dataset_path}.graph
    wget http://data.law.di.unimi.it/webdata/uk-union-2006-06-2007-05/uk-union-2006-06-2007-05-underlying.properties -O ${dataset_path}.properties
    echo "unzip the ${dataset}..."
    git clone https://github.com/Mogami95/graph-xll.git
    cd graph-xll
    java -cp "lib/*:bin" BV2Ascii ${dataset_path} > ${dataset_path}
    echo "delete the download temporary file..." 
    rm -f ${dataset_path}.graph ${dataset_path}.properties
    cd .. && rm -rf graph-xll
else
    echo "the ${dataset} dataset already exists, skip the download process"
fi  

echo "successfully down!"

