In this document, we will briefly introduce our framework `SOWalker` and the instructions for making it run.

# SOWalker

`SOWalker` is an  I/O-optimized out-of-core graph processing system for second-order random walks.

## Introduction

Random walks serve as a powerful tool for extracting information that exists in a wide variety of real-world scenarios. Different from the traditional first-order random walk, the second-order random walk considers recent walk history in selecting the next stop, which facilitates to model higher-order structures in real-world data. To meet the scalability of random walks, researchers have developed many out-of-core graph processing systems based on a single machine. However, the main focus of out-of-core graph processing systems is to support first-order random walks, which no longer perform well for second-order random walks. 

In this paper, we propose an I/O-optimized out-of-core graph processing system for second-order random walks, called SOWalker. First, we propose a 2D-block walk management to avoid loading useless walks and eliminate useless walk I/Os. Second, we develop a benefit-aware I/O model to load multiple blocks with the maximum accumulated updatable walks, so as to improve the I/O utilization. Finally, we adopt a block set-oriented walk updating scheme, which allows each walk to move as many steps as possible in the loaded block set, thus significantly boosting the walk updating rate. Compared with two state-of-the-art random walk systems, GraphWalker and GraSorw, SOWalker yields significant performance speedups (up to 10.2×).

## Publication



## Quick start

### [Option]Install OR-tools

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

### Compiling

Using the following commands, one can easily compile the `SOWalker`, the compiled applications are located at `bin/test`

```
$ git clone https://github.com/tzphh/SOWalker.git
$ cd SOWalker
$ make
```

### Dataset preparing

```
$ mkdir Dataset && cd Dataset
$ wget https://snap.stanford.edu/data/soc-LiveJournal1.txt.gz
$ gunzip soc-LiveJournal1.txt.gz
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

### Run Node2vec

```
./bin/test/node2vec /Dataset/soc-LiveJournal1.txt c_update dynamic length 20 walkpersource 1 p 2.0 q 0.5 schedule sa
```

