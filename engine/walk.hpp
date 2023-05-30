#ifndef _GRAPH_WALK_H_
#define _GRAPH_WALK_H_

#include <algorithm>
#include "api/types.hpp"
#include "api/graph_buffer.hpp"
#include "util/hash.hpp"
#include "cache.hpp"

class block_desc_manager_t {
private:
    int desc;
public:
    block_desc_manager_t(const std::string&& file_name) {
        desc = open(file_name.c_str(), O_RDWR | O_CREAT | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
    }
    ~block_desc_manager_t() {
        if(desc > 0) close(desc);
    }
    int get_desc() const { return desc; }
};

class graph_walk {
public:
    std::string base_name;  /* the dataset base name, indicate the walks store path */
    size_t blocksize;
    vid_t nvertices;
    vid_t minvert;
    bid_t nblocks, totblocks;
    tid_t nthreads;
    graph_driver *global_driver;
    std::vector<hid_t> maxhops;                         /* record the block has at least `maxhops` to finished */
    graph_buffer<walker_t> walks;          /* the walks in current block */
    graph_buffer<walker_t> **block_walks;  /* the walk resident in memroy */
    std::vector<std::vector<wid_t>> block_nmwalk;       /* record each block number of walks in memroy */
    std::vector<std::vector<wid_t>> block_ndwalk;       /* record each block number of walks in disk */
    graph_block *global_blocks;
#ifdef COUNT_REUSED
    std::vector<vid_t> reused;
#endif
#ifdef IO_UTE
    eid_t current_steps;
#endif
    // BloomFilter *bf;
    graph_walk(graph_config& conf, graph_driver& driver, graph_block &blocks) {
        base_name = conf.base_name;
        nvertices = conf.nvertices;
        minvert =conf.min_vert;
        nthreads = conf.nthreads;
        blocksize = conf.blocksize;
        global_driver = &driver;
        global_blocks = &blocks;
        nblocks = global_blocks->nblocks;

        totblocks = nblocks * nblocks;
        maxhops.resize(totblocks, 0);
        walks.alloc(conf.max_nthreads * MAX_TWALKS * 5);
#ifdef COUNT_REUSED
        reused.resize(nblocks,0);
#endif
#ifdef IO_UTE
        current_steps=0;
#endif
        block_nmwalk.resize(totblocks);
        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            block_nmwalk[blk].resize(nthreads);
            std::fill(block_nmwalk[blk].begin(), block_nmwalk[blk].end(), 0);
        }

        block_ndwalk.resize(totblocks);
        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            block_ndwalk[blk].resize(nthreads);
            std::fill(block_ndwalk[blk].begin(), block_ndwalk[blk].end(), 0);
        }

        block_walks = (graph_buffer<walker_t> **)malloc(totblocks * sizeof(graph_buffer<walker_t> *));
        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            block_walks[blk] = (graph_buffer<walker_t> *)malloc(nthreads * sizeof(graph_buffer<walker_t>));
            for (tid_t tid = 0; tid < nthreads; tid++)
            {
                block_walks[blk][tid].alloc(MAX_TWALKS);
            }
        }

        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            std::string walk_name = get_walk_name(base_name, blocksize, blk);
            if(test_exists(walk_name)) unlink(walk_name.c_str());
        }

    }

    ~graph_walk()
    {
        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            for (tid_t tid = 0; tid < nthreads; tid++)
            {
                block_walks[blk][tid].destroy();
            }
            free(block_walks[blk]);
        }
        free(block_walks);

        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            std::string walk_name = get_walk_name(base_name, blocksize, blk);
            if(test_exists(walk_name)) unlink(walk_name.c_str());
        }
        walks.destroy();

        // if(bf) delete bf;
    }

    void move_walk(const walker_t &walker)
    {
        tid_t t = static_cast<vid_t>(omp_get_thread_num());
        bid_t pblk = WALKER_PREV_BLOCK(walker), cblk = WALKER_CUR_BLOCK(walker);
        bid_t blk = pblk * nblocks + cblk;
        if(block_walks[blk][t].full()) {
            persistent_walks(blk, t);
        }

        block_nmwalk[blk][t] += 1;
        block_walks[blk][t].push_back(walker);
    }

    void persistent_walks(bid_t blk, tid_t t)
    {
        block_ndwalk[blk][t] += block_walks[blk][t].size();
        block_nmwalk[blk][t] -= block_walks[blk][t].size();
        appendfile(get_walk_name(base_name, blocksize, blk), block_walks[blk][t].buffer_begin(), block_walks[blk][t].size());
        block_walks[blk][t].clear();
    }

    wid_t nblockwalks(bid_t blk)
    {
        wid_t walksum = 0;
        for (tid_t t = 0; t < nthreads; t++)
        {
            walksum += block_nmwalk[blk][t] + block_ndwalk[blk][t];
        }
        return walksum;
    }

    wid_t nmwalks(bid_t exec_block)
    {
        wid_t walksum = 0;
        for (tid_t t = 0; t < nthreads; t++)
        {
            walksum += block_nmwalk[exec_block][t];
        }
        return walksum;
    }

    wid_t ndwalks(bid_t exec_block)
    {
        wid_t walksum = 0;
        for (tid_t t = 0; t < nthreads; t++)
        {
            walksum += block_ndwalk[exec_block][t];
        }
        return walksum;
    }

    wid_t ncwalks(graph_cache *cache)
    {
        wid_t walk_sum = 0;
        for (bid_t p = 0; p < cache->ncblock; p++)
        {
            if (cache->cache_blocks[p].block != NULL && cache->cache_blocks[p].block->status != INACTIVE)
            {
                bid_t blk = cache->cache_blocks[p].block->blk;
                walk_sum += this->nblockwalks(blk);
            }
        }
        return walk_sum;
    }

    wid_t nwalks()
    {
        wid_t walksum = 0;
        for (bid_t blk = 0; blk < totblocks; blk++)
        {
            walksum += this->nblockwalks(blk);
        }
        return walksum;
    }

    size_t load_memory_walks(bid_t exec_block) {

        /* load in memory walks */
        for (tid_t t = 0; t < nthreads; t++)
        {
            if (block_walks[exec_block][t].empty())
                continue;
            for (wid_t w = 0; w < block_walks[exec_block][t].size(); w++)
            {
                walks.push_back(block_walks[exec_block][t][w]);
            }
        }

        /* clear memory walks */
        std::fill(block_nmwalk[exec_block].begin(), block_nmwalk[exec_block].end(), 0);
        for (tid_t t = 0; t < nthreads; t++)
        {
            block_walks[exec_block][t].clear();
        }
        return 0;
    }

    size_t load_disk_walks(bid_t exec_block, wid_t walk_cnt, wid_t loaded_walks) {
        walks.clear();
        block_desc_manager_t block_desc(get_walk_name(base_name, blocksize, exec_block));
        global_driver->load_walk(block_desc.get_desc(), walk_cnt, loaded_walks, walks);
        return walks.size();
    }

    void dump_walks(bid_t exec_block)
    {
        std::fill(block_ndwalk[exec_block].begin(), block_ndwalk[exec_block].end(), 0);
        block_desc_manager_t block_desc(get_walk_name(base_name, blocksize, exec_block));
        ftruncate(block_desc.get_desc(), 0);
    }

    bool test_finished_walks()
    {
        return this->nwalks() == 0;
    }

    bool test_finished_cache_walks(graph_cache *cache)
    {
        return this->ncwalks(cache) == 0;
    }

    bid_t max_walks_block()
    {
        wid_t max_walks = 0;
        bid_t blk = 0;
        for (bid_t p = 0; p < totblocks; p++)
        {
            wid_t walk_cnt = this->nblockwalks(p);
            if (max_walks < walk_cnt)
            {
                max_walks = walk_cnt;
                blk = p;
            }
        }
        return blk;
    }

    bid_t max_hops_block()
    {
        hid_t walk_hop = 0;
        bid_t blk = 0;
        for (bid_t p = 0; p < totblocks; p++)
        {
            if (maxhops[p] > walk_hop)
            {
                walk_hop = maxhops[p];
                blk = p;
            }
        }
        if (this->nblockwalks(blk) == 0)
            return max_walks_block();
        return blk;
    }

    bid_t choose_block(float prob)
    {
        float cc = (float)rand() / RAND_MAX;
        if (cc < prob)
            return max_hops_block();
        else
            return max_walks_block();
    }

    wid_t block_active_walks(bid_t blk) {
        wid_t walks_cnt = 0;
        for (bid_t p = 0; p < nblocks; p++)
        {
            walks_cnt += this->nblockwalks(p * nblocks + blk);
            walks_cnt += this->nblockwalks(blk * nblocks + p);
        }
        walks_cnt -= this->nblockwalks(blk * nblocks + blk);
        return walks_cnt;
    }
};

#endif
