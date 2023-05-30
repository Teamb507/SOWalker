#ifndef _GRAPH_PRECOMPUTE_H_
#define _GRAPH_PRECOMPUTE_H_

#include <string>
#include <vector>
#include <numeric>
#include <queue>
#include <omp.h>
#include <algorithm>
#include <unordered_set>
#include "api/types.hpp"
#include "util/util.hpp"
#include "util/io.hpp"
#include "util/hash.hpp"

/* the block structure used for precompute */
struct pre_block_t {
    vid_t start_vert, nverts;
    eid_t start_edge, nedges;
    eid_t *beg_pos;
    vid_t *csr;
    real_t *weights;

    pre_block_t() {
        beg_pos = NULL;
        csr = NULL;
        weights = NULL;
    }
    ~pre_block_t() {
        if(beg_pos) free(beg_pos);
        if(csr) free(csr);
        if(weights) free(weights);
    }
};


static void sort_block_vertex_neighbors(const pre_block_t& block, vid_t *new_csr, real_t *new_weights)
{
    vid_t *csr_start = block.csr;
    auto cmp = [&csr_start](eid_t u, eid_t v) { return csr_start[u] < csr_start[v]; };
    omp_set_num_threads(omp_get_max_threads());

#pragma omp parallel for schedule(static)
    for(vid_t vertex = 0; vertex < block.nverts; ++vertex) {
        eid_t adj_head = block.beg_pos[vertex] - block.start_edge, adj_tail = block.beg_pos[vertex + 1] - block.start_edge;
        std::vector<eid_t> index(adj_tail - adj_head);
        std::iota(index.begin(), index.end(), adj_head);
        std::sort(index.begin(), index.end(), cmp);
        for(eid_t off = adj_head; off < adj_tail; ++off) {
            new_csr[off] = block.csr[index[off - adj_head]];
            if(new_weights != nullptr) {
                new_weights[off] = block.weights[index[off - adj_head]];
            }
        }
    }
}

void sort_vertex_neighbors(const std::string &base_name, size_t blocksize, bool weighted)
{
    std::string vert_block_name = get_vert_blocks_name(base_name, blocksize);
    std::string edge_block_name = get_edge_blocks_name(base_name, blocksize);
    std::string weight_name = get_weights_name(base_name);
    std::string beg_pos_name = get_beg_pos_name(base_name);
    std::string csr_name = get_csr_name(base_name);

    std::vector<vid_t> vblocks = load_graph_blocks<vid_t>(vert_block_name);
    std::vector<eid_t> eblocks = load_graph_blocks<eid_t>(edge_block_name);

    int vertdesc = open(beg_pos_name.c_str(), O_RDONLY);
    int edgedesc = open(csr_name.c_str(), O_RDWR);
    int whtdesc = 0;
    if(weighted) {
        whtdesc = open(weight_name.c_str(), O_RDWR);
    }
    assert(vertdesc > 0 && edgedesc > 0);
    bid_t nblocks = vblocks.size() - 1;
    logstream(LOG_INFO) << "load vblocks and eblocks successfully, block count : " << nblocks << std::endl;
    pre_block_t block;
    logstream(LOG_INFO) << "start to sort the vertex neighbors, nblocks = " << nblocks << std::endl;

    vid_t *new_csr = nullptr;
    real_t *new_weights = nullptr;
    for (bid_t blk = 0; blk < nblocks; blk++)
    {
        block.nverts = vblocks[blk + 1] - vblocks[blk];
        block.nedges = eblocks[blk + 1] - eblocks[blk];
        block.start_vert = vblocks[blk];
        block.start_edge = eblocks[blk];

        block.beg_pos = (eid_t *)realloc(block.beg_pos, (block.nverts + 1) * sizeof(eid_t));
        block.csr = (vid_t *)realloc(block.csr, block.nedges * sizeof(vid_t));
        if(weighted) {
            block.weights = (real_t *)realloc(block.weights, block.nedges * sizeof(real_t));
        }

        load_block_range(vertdesc, block.beg_pos, (block.nverts + 1), block.start_vert * sizeof(eid_t));
        load_block_range(edgedesc, block.csr, block.nedges, block.start_edge * sizeof(vid_t));
        if(weighted) {
            load_block_range(whtdesc, block.weights, block.nedges, block.start_edge * sizeof(real_t));
        }

        new_csr = (vid_t *)realloc(new_csr, block.nedges * sizeof(vid_t));
        if(weighted) {
            new_weights = (real_t *)realloc(new_weights, block.nedges * sizeof(real_t));
        }

        sort_block_vertex_neighbors(block, new_csr, new_weights);
        dump_block_range(edgedesc, new_csr, block.nedges, block.start_edge * sizeof(vid_t));
        if(weighted) {
            dump_block_range(whtdesc, new_weights, block.nedges, block.start_edge * sizeof(real_t));
        }
        logstream(LOG_INFO) << "finish sort vertex neighbors for block = " << blk << std::endl;
    }

    if(new_csr) free(new_csr);
    if(new_weights) free(new_weights);
}

#endif
