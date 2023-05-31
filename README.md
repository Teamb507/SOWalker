In this document, we will briefly introduce our framework `SOWalker` and the instructions for making it run.

# SOWalker

`SOWalker` is an  I/O-optimized out-of-core graph processing system for second-order random walks.

## Introduction

Random walks serve as a powerful tool for extracting information that exists in a wide variety of real-world scenarios. Different from the traditional first-order random walk, the second-order random walk considers recent walk history in selecting the next stop, which facilitates to model higher-order structures in real-world data. To meet the scalability of random walks, researchers have developed many out-of-core graph processing systems based on a single machine. However, the main focus of out-of-core graph processing systems is to support first-order random walks, which no longer perform well for second-order random walks. 

In this paper, we propose an I/O-optimized out-of-core graph processing system for second-order random walks, called SOWalker. First, we propose a 2D-block walk management to avoid loading useless walks and eliminate useless walk I/Os. Second, we develop a benefit-aware I/O model to load multiple blocks with the maximum accumulated updatable walks, so as to improve the I/O utilization. Finally, we adopt a block set-oriented walk updating scheme, which allows each walk to move as many steps as possible in the loaded block set, thus significantly boosting the walk updating rate. Compared with two state-of-the-art random walk systems, GraphWalker and GraSorw, SOWalker yields significant performance speedups (up to 10.2×).

## Quick start

### Docker image

You can use these  instruction to get our image.

```
docker pull tzphh/sowalker:v3
docker run -it  tzphh/sowalker:v3  /bin/bash
```

The image includes:

- Source code.Both SoWalker and GraphWalker codes are provided.
- Scripts in `script/`. Scripts are provided for running experiments.
- Plotting scripts in  `plot/`These were used for plotting the figures in the SoWalker paper.

#### Github repository

##### Install OR-tools

- ortools : https://developers.google.com/optimization/install/cpp/source_linux

```
sudo apt-get -y install git wget pkg-config build-essential cmake autoconf libtool zlib1g-dev lsb-release
git clone -b v9.2 https://github.com/google/or-tools
cd or-tools
cmake -S . -B build -DBUILD_DEPS=ON
cmake --build build --config Release --target install -v
echo "export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:/usr/local/lib/\"" >> ~/.bashrc
source ~/.bashrc
```

##### Git clone

```
$ git clone https://github.com/tzphh/SOWalker.git
```

### Compiling

Using the following commands, one can easily compile the `SOWalker`, the compiled applications are located at `bin/test`

```
$ make
```

### Configurations

All of the applications above share a common set of configurations, which we list below:

```
- dataset:       the dataset path
- weighted:      whether the dataset is weighted
- sorted:        whether the vertex neighbors is sorted
- skip:          whether to skip preprocessing
- c_update       whether to update walks continuously
- blocksize:     the size of each block
- nthreads:      the number of threads to walk
- dynamic:       whether the blocksize is dynamic, according to the number of walks
- memory_size:   the size(GB) of memory
- max_iter:      the maximum number of iteration for simulated annealing scheduler
- walkpersource: the number of walks for each vertex
- length:        the number of step for each walk
- p:             node2vec parameter
- q:             node2vec parameter
- schedule:      scheduling strategy
- h:             print this message
```

### Dataset preparing

#### Twitter, Friendster and UK-Union graphs

To download the Twitter, Friendster and UK-Union graphs:

```
./script/download_twitter.sh
./script/download_friendster.sh
./script/download_uk_union.sh
```

#### Kron30 graph

Kron30 is a synthetic graph generated with the Graph500 generator. https://graph500.org/. 

#### CrawlWeb graph

CrawlWeb can be downloaded from 'The 2012 common crawl graph' available at http://webdatacommons.org."



## Discussion

### Figure 5 (Execution time) and Figure 6 (Block I/O time)

 In this section of the experiment, we also compared GraSORW. We directly used the code provided by the author, which can be downloaded from https://github.com/DuoLife-QNL/GraSorw. You can easily run and modify the code according to the readme file. After that, copy the generated graphwalker_metrics.txt file to `/log` and rename it as grasorw_node2vec_log and grasorw_pagerank_log respectively.

The complete experiment can be executed using the `./script/evaluation.sh`. Please note that you need to modify the dataset path accordingly.After executing this script command，you can get log file in`/log` and plot the figures in the  SOWalker paper .


###  Figure 7 (I/O utilization and walk updating rate)

Firstly,modify the Makefile by adding the compilation option `-D IO_UTE` and rebuild.

In our paper, we define the **I/O utilization rate** as the number of walk steps divided by the number of edges in a loaded block. Additionally, the **walk updating rate** is defined as the sum of the number of walk steps in a loaded block divided by the total steps needed to walk (e.g., |V| * walkpersource * length in node2vec), which is a constant.

Use `./script/io_utilization.py` to process the log file and plot the figures in the  SOWalker paper.You can get two files,`sowalker_log `and `graphwalker_log`,which indicate the connection between updated steps and Block I/O.

### Table 3 (Comparisons of Scheduling Models)

The parameter `schedule` is used to specify the scheduling models, where `random` means the Random model, `greedy` means the Max-m model, `lp` means the Exact model, and `sa` means the BA model.

The complete experiment can be executed using the `./script/scheduling.sh`.After executing this script command，you can get log file in`/log`.You can easily get the data in the table by reading the log.

### Figure 8 (Impact of Block Size)

The parameter `blocksize` is used to specify the block size in bytes. The memory size is set to 4GB, which is controlled by the `MEMORY_CACHE` in `api/constants.hpp`.

The complete experiment can be executed using the `./script/blocksize.sh`. Please note that you need to modify the dataset path accordingly.After executing this script command，you can get log file in`/log `and plot the figures in the  SOWalker paper.
