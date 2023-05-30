#ifndef _SECOND_ORDER_H_
#define _SECOND_ORDER_H_

#include <omp.h>
#include <unordered_set>
#include <functional>
#include "api/types.hpp"
#include "engine/walk.hpp"
#include "util/timer.hpp"
#include "util/hash.hpp"
#include <fstream>
#include <mutex>    
#include <climits>

class second_order_app_t
{
public:
    wid_t _walkpersource;
    hid_t _hops;
    bool continue_update;
    std::mutex mtx;   
    second_order_app_t(wid_t nwalks, hid_t steps, bool c_update = true)
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


class node2vec_app_t : public second_order_app_t {
private:
    real_t p, q;
public:
    node2vec_app_t(wid_t nwalks, hid_t steps, real_t param_p, real_t param_q) : second_order_app_t(nwalks, steps, true) 
    {
        p = param_p;
        q = param_q;
    }

    node2vec_app_t(wid_t nwalks, hid_t steps, bool c_update, real_t param_p, real_t param_q) : second_order_app_t(nwalks, steps, c_update)
    {
        p = param_p;
        q = param_q;
    }

    wid_t update_walk(walker_t &walker, graph_cache *cache, graph_walk *walk_manager, RandNum *seed)
    { 
        vid_t cur_vertex = WALKER_POS(walker), prev_vertex = WALKER_PREVIOUS(walker);
        hid_t hop = WALKER_HOP(walker);
        bid_t cur_blk = WALKER_CUR_BLOCK(walker), prev_blk = WALKER_PREV_BLOCK(walker);
        bid_t cur_cache_index = (*(walk_manager->global_blocks))[cur_blk].cache_index, prev_cache_index = (*(walk_manager->global_blocks))[prev_blk].cache_index;
        bid_t nblocks = walk_manager->global_blocks->nblocks;
        //assert(cur_cache_index != nblocks && prev_cache_index != nblocks);
        if(cur_cache_index == nblocks || prev_cache_index == nblocks){
            return 0;
        }
        wid_t run_step = 0;
        while (cur_cache_index != nblocks && hop < this->_hops)
        {
            cache_block *cur_block = &(cache->cache_blocks[cur_cache_index]);
            cache_block *prev_block = &(cache->cache_blocks[prev_cache_index]);

            vid_t start_vertex = cur_block->block->start_vert, off = cur_vertex - start_vertex;
            vid_t prev_start_vertex = prev_block->block->start_vert, prev_off = prev_vertex - prev_start_vertex;

            eid_t adj_head = cur_block->beg_pos[off] - cur_block->block->start_edge, adj_tail = cur_block->beg_pos[off + 1] - cur_block->block->start_edge;
            eid_t prev_adj_head = prev_block->beg_pos[prev_off] - prev_block->block->start_edge, prev_adj_tail = prev_block->beg_pos[prev_off + 1] - prev_block->block->start_edge;

            vid_t next_vertex = 0;

            eid_t deg = adj_tail - adj_head;
            if (deg == 0){       
                hop=this->_hops-1;
            }
            else
            {
                real_t max_val = std::max(1.0 / p, std::max(1.0 / q, 1.0));
                real_t min_val = std::min(1.0 / p, std::min(1.0 / q, 1.0));
                bool accept = false;
                size_t rand_pos = 0;
                while (!accept)
                {
                    real_t rand_val = seed->dRand() * max_val;
                    rand_pos = seed->iRand(static_cast<uint32_t>(deg));
                    if (rand_val <= min_val)
                    {
                        accept = true;
                        break;
                    }
                    if (cur_block->csr[adj_head + rand_pos] == prev_vertex)
                    {
                        if (rand_val < 1.0 / p)
                            accept = true;
                    }
                    else if (std::binary_search(prev_block->csr + prev_adj_head, prev_block->csr + prev_adj_tail, cur_block->csr[adj_head + rand_pos]))
                    {
                        if (rand_val < 1.0)
                            accept = true;
                    }
                    else
                    {
                        if (rand_val < 1.0 / q)
                            accept = true;
                    }
                }
                assert(rand_pos<deg);
                next_vertex = cur_block->csr[adj_head + rand_pos];
            }
            prev_vertex = cur_vertex;
            cur_vertex = next_vertex;
            prev_blk = cur_blk;
            hop++;
            run_step++;
            prev_cache_index = cur_cache_index;
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
            walker_t next_walker = walker_makeup(WALKER_ID(walker), WALKER_SOURCE(walker), prev_vertex, cur_vertex, hop, cur_blk, prev_blk);
            walk_manager->move_walk(next_walker);
        }
#ifdef IO_UTE       
        mtx.lock(); 
        walk_manager->current_steps+=run_step;
        mtx.unlock();
#endif
        return run_step;
    }
};

class autoregreesive_app_t : public second_order_app_t
{
private:
    real_t alpha;

public:
    autoregreesive_app_t(wid_t nwalks, hid_t steps, real_t param_alpha) : second_order_app_t(nwalks, steps, true)
    {
        alpha = param_alpha;
    }

    autoregreesive_app_t(wid_t nwalks, hid_t steps, bool c_update, real_t param_alpha) : second_order_app_t(nwalks, steps, c_update)
    {
        alpha = param_alpha;
    }

    wid_t update_walk(walker_t &walker, graph_cache *cache, graph_walk *walk_manager, RandNum *seed)
    {
        vid_t cur_vertex = WALKER_POS(walker), prev_vertex = WALKER_PREVIOUS(walker);
        hid_t hop = WALKER_HOP(walker);
        bid_t cur_blk = WALKER_CUR_BLOCK(walker), prev_blk = WALKER_PREV_BLOCK(walker);
        bid_t cur_cache_index = (*(walk_manager->global_blocks))[cur_blk].cache_index, prev_cache_index = (*(walk_manager->global_blocks))[prev_blk].cache_index;
        bid_t nblocks = walk_manager->global_blocks->nblocks;
        //assert(cur_cache_index != nblocks && prev_cache_index != nblocks);
        if(cur_cache_index == nblocks || prev_cache_index == nblocks){
            return 0;
        }
        wid_t run_step = 0;
        while (cur_cache_index != nblocks && hop < this->_hops)
        {
            cache_block *cur_block = &(cache->cache_blocks[cur_cache_index]);
            cache_block *prev_block = &(cache->cache_blocks[prev_cache_index]);

            vid_t start_vertex = cur_block->block->start_vert, off = cur_vertex - start_vertex;
            vid_t prev_start_vertex = prev_block->block->start_vert, prev_off = prev_vertex - prev_start_vertex;

            eid_t adj_head = cur_block->beg_pos[off] - cur_block->block->start_edge, adj_tail = cur_block->beg_pos[off + 1] - cur_block->block->start_edge;
            eid_t prev_adj_head = prev_block->beg_pos[prev_off] - prev_block->block->start_edge, prev_adj_tail = prev_block->beg_pos[prev_off + 1] - prev_block->block->start_edge;

            vid_t next_vertex = 0;

            eid_t deg = adj_tail - adj_head, prev_deg = prev_adj_tail - prev_adj_head;
            eid_t max_deg = std::max(deg, prev_deg);
            if (deg == 0) next_vertex = seed->iRand(walk_manager->nvertices);
            else
            {
                std::vector<real_t> adj_weights(deg + 1, 0.0);
                std::unordered_set<vid_t> prev_neighbors(prev_block->csr + prev_adj_head, prev_block->csr + prev_adj_tail);
                for(size_t index = 0; index < deg; ++index) {
                    real_t wht = 0.0;
                    if(cur_block->csr[adj_head + index] == prev_vertex) {
                        wht = (1.0 - alpha) / deg;
                    }else if(prev_neighbors.find(cur_block->csr[adj_head + index]) != prev_neighbors.end()) {
                        wht = (1.0 - alpha) / deg + alpha / prev_deg;
                    }else {
                        wht = (1.0 - alpha) / deg;
                    }
                    adj_weights[index + 1] = adj_weights[index] + wht * max_deg;
                }

                real_t rand_val = seed->dRand() * adj_weights[deg];
                size_t pos = std::upper_bound(adj_weights.begin(), adj_weights.end(), rand_val) - adj_weights.begin();
                next_vertex = cur_block->csr[adj_head + pos - 1];
            }
            prev_vertex = cur_vertex;
            cur_vertex = next_vertex;
            prev_blk = cur_blk;
            hop++;
            run_step++;
            prev_cache_index = cur_cache_index;
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
            walker_t next_walker = walker_makeup(WALKER_ID(walker), WALKER_SOURCE(walker), prev_vertex, cur_vertex, hop, cur_blk, prev_blk);
            walk_manager->move_walk(next_walker);
        }
        return run_step;
    }
};

#endif
