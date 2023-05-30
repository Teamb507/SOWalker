# the download dataset name
dataset=twitter_tv.net

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
    wget https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.00.gz -O ${dataset_path}.00.gz
    wget https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.01.gz -O ${dataset_path}.01.gz
    wget https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.02.gz -O ${dataset_path}.02.gz
    wget https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.03.gz -O ${dataset_path}.03.gz
    echo "unzip the ${dataset}.gz..."
    gunzip -c ${dataset_path}.00.gz ${dataset_path}.01.gz ${dataset_path}.02.gz ${dataset_path}.03.gz > ${dataset_path}
    echo "delete the download temporary file..."
    rm -f ${dataset_path}.00.gz ${dataset_path}.01.gz ${dataset_path}.02.gz ${dataset_path}.03.gz
else
    echo "the ${dataset} dataset already exists, skip the download process"
fi

echo "successfully down!"

