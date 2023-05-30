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

### Figure 5 (Execution time) and Figure 6 (Block I/O time)

#### Evaluate SOWalker

```
./bin/test/node2vec /data/tw/twitter_tv.net c_update length 80 walkpersource 1 p 2.0 q 0.5 schedule sa skip
```

where 

* `c_update` means adopting block set-oriented walk updating scheme;

* `length` means walk length;

* `walkpersource` means the number of walks starting from each vertex;

* `p` and `q` are the parameters in node2vec;

* `schedule sa` means adopting the benefit-aware I/O model according to the simulated annealing algorithm;

* `skip` means the preprocessing step is skipped if the text format of the graph has been converted to csr format.

#### Output

The results are saved in `./randgraph_metrics.txt`, which contains the following content.

```
[node2vec_walkpersource_1_steps_80_dataset_/data/tw/twitter_tv.net:]
.act_simulated_annealing_scheduler_swap_blocks=0.003366
.act_simulated_annealing_scheduler_swap_blocks.count=101
.act_simulated_annealing_scheduler_swap_blocks.min=0.000024
.act_simulated_annealing_scheduler_swap_blocks.max=0.000051
.act_simulated_annealing_scheduler_swap_blocks.avg=0.000033
.app=node2vec_walkpersource_1_steps_80_dataset_/data/tw/twitter_tv.net
.c_update=1
.c_update.count=1
.c_update.min=1
.c_update.max=1
.c_update.avg=1.000000
.exec_block_walk=110.746530
.exec_block_walk.count=1129
.exec_block_walk.min=0.000010
.exec_block_walk.max=0.169414
.exec_block_walk.avg=0.098093
.load_block_info=39.485015
.load_block_info.count=113
.load_block_info.min=0.177151
.load_block_info.max=0.691122
.load_block_info.avg=0.349425
.nblocks=6
.ncblocks=4
.run_app=161.204929
.run_app.count=1
.run_app.min=161.204929
.run_app.max=161.204929
.run_app.avg=161.204929
.scheduler=sa
.simulated_annealing_scheduler_swap_blocks=39.496988
.simulated_annealing_scheduler_swap_blocks.count=101
.simulated_annealing_scheduler_swap_blocks.min=0.000062
.simulated_annealing_scheduler_swap_blocks.max=2.380189
.simulated_annealing_scheduler_swap_blocks.avg=0.391059
```

where 

* `run_app` means the execution time;

* `load_block_info` means the block I/O time.

The complete experiment can be executed using the `./script/evaluation.sh`. Please note that you need to modify the dataset path accordingly.


###  Figure 7 (I/O utilization and walk updating rate)

Modify the Makefile by adding the compilation option `-D IO_UTE` and rebuild.

#### Run node2vec

```
./bin/test/node2vec /data/tw/twitter_tv.net c_update length 80 walkpersource 1 p 2.0 q 0.5 schedule sa skip >log
```

#### Output

In our paper, we define the **I/O utilization rate** as the number of walk steps divided by the number of edges in a loaded block. Additionally, the **walk updating rate** is defined as the sum of the number of walk steps in a loaded block divided by the total steps needed to walk (e.g., |V| * walkpersource * length in node2vec), which is a constant.

Use `./script/io_utilization.py` to process the log file and generate `log.csv` file. The first column is the loaded block id, the second column is the number of edges in a loaded block, and the third column is the number of walk steps.

**'log.csv' file sample**

```
block_id	nedges	    update_steps
10	        126871019	38279683
0	        134217559	14820897
9	        134214279	29615230
8	        134212460	31539306
...
7	        134157594	24629446
```

The complete experiment can be executed using the `./script/io_utilization.sh`.

### Table 3 (Comparisons of Scheduling Models)

#### Run node2vec on UK

The parameter `schedule` is used to specify the scheduling models, where `random` means the Random model, `greedy` means the Max-m model, `lp` means the Exact model, and `sa` means the BA model. 

```
./bin/test/node2vec /data/uk/uk-union.txt  length 80 walkpersource 1 p 2.0 q 0.5 schedule lp skip
```

#### Output

The execution results are saved in `./randgraph_metrics.txt`, where

* `run_app` means the execution time;

* `load_block_info` means the block I/O time;

* `load_block_info.count` means the block I/O number;

* `lp_solve_schedule` and `simulated_annealing_scheduler_swap_blocks` means the computation time of Exact and BA, respectively.

The complete experiment can be executed using the `./script/scheduling.sh`.

### Figure 8 (Impact of Block Size)

The parameter `blocksize` is used to specify the block size in bytes. The memory size is set to 4GB, which is controlled by the `MEMORY_CACHE` in `api/constants.hpp`.

```
./bin/test/node2vec /data/uk/uk-union.txt  c_update blocksize 1073741824 length 80 walkpersource 1 p 2.0 q 0.5  schedule sa
```

The execution results are saved in `./randgraph_metrics.txt`, where `run_app` means the execution time.

The complete experiment can be executed using the `./script/blocksize.sh`.
