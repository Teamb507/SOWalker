#ifndef _FIRST_ORDER_H_
#define _FIRST_ORDER_H_
#include <climits>
#include <omp.h>
#include <unordered_set>
#include <functional>
#include "api/types.hpp"
#include "engine/walk.hpp"
#include "util/timer.hpp"
#include "util/hash.hpp"
#include <fstream>
#include <mutex>    

class first_order_app_t
{
public:
    wid_t _walkpersource;
    hid_t _hops;
    bool continue_update;
    first_order_app_t(wid_t nwalks, hid_t steps, bool c_update = true)
    {
        _walkpersource = nwalks;
        _hops = steps;
        continue_update = c_update;
    }
    void prologue(graph_walk *walk_manager, std::function<void(graph_walk *walk_manager)> init_func = nullptr)
    {
        if(init_func) init_func(walk_manager);
    }

    virtual wid_t update_walk( walker_t &walker, graph_cache *cache, graph_walk *walk_manager, RandNum* seed)
    {
        return 0;
    }

    void epilogue()
    {

    }

    wid_t get_numsources() { return _walkpersource; }
    hid_t get_hops() { return _hops; }
};


class deepwalk_app_t : public first_order_app_t {
private:

public:
    deepwalk_app_t(wid_t nwalks, hid_t steps) : first_order_app_t (nwalks, steps, true) {}
    deepwalk_app_t(wid_t nwalks, hid_t steps, bool c_update) : first_order_app_t(nwalks, steps, c_update) {}

    wid_t update_walk(walker_t &walker, graph_cache *cache, graph_walk *walk_manager, RandNum *seed)
    {
        vid_t cur_vertex = WALKER_POS(walker);
        hid_t hop = WALKER_HOP(walker);
        bid_t cur_blk = WALKER_CUR_BLOCK(walker);
        bid_t cur_cache_index = (*(walk_manager->global_blocks))[cur_blk].cache_index;
        bid_t nblocks = walk_manager->global_blocks->nblocks;
        wid_t run_step = 0;
        while (cur_cache_index != nblocks && hop < this->_hops)
        {
            cache_block *cur_block = &(cache->cache_blocks[cur_cache_index]);
            vid_t start_vertex = cur_block->block->start_vert, off = cur_vertex - start_vertex;
            eid_t adj_head = cur_block->beg_pos[off] - cur_block->block->start_edge, adj_tail = cur_block->beg_pos[off + 1] - cur_block->block->start_edge;
            vid_t next_vertex = 0;
            eid_t deg = adj_tail - adj_head;
            if (deg == 0){
                hop=this->_hops-1;
            }
            else
            {
                size_t rand_pos = seed->iRand(static_cast<uint32_t>(deg));
                next_vertex = cur_block->csr[adj_head + rand_pos];
            }
            cur_vertex = next_vertex;
            hop++;
            run_step++;
            if (!(cur_vertex >= cur_block->block->start_vert && cur_vertex < cur_block->block->start_vert + cur_block->block->nverts))
            {
                cur_blk = walk_manager->global_blocks->get_block(cur_vertex);
                cur_cache_index = (*(walk_manager->global_blocks))[cur_blk].cache_index;
                if (!continue_update)
                    break;
            }
        }
        if (hop < this->_hops)
        {
            walker_t next_walker = walker_makeup(WALKER_ID(walker), WALKER_SOURCE(walker), 0, cur_vertex, hop, cur_blk, 0);
            walk_manager->move_walk(next_walker);
        }
        return run_step;
    }
};

#endif
