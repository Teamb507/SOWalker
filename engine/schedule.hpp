#ifndef _GRAPH_SCHEDULE_H_
#define _GRAPH_SCHEDULE_H_

#include <limits>
#include <string>
#include <algorithm>
#include <utility>
#include <queue>
#include <random>
#include <numeric>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "cache.hpp"
#include "config.hpp"
#include "driver.hpp"
#include "walk.hpp"
#include "util/util.hpp"
#include "util/io.hpp"
#include "util/lp_solver.hpp"
#include "metrics/metrics.hpp"
#include "metrics/reporter.hpp"
#define TEMPERATURE_COOLING  
template<typename value_t>
struct rank_compare {
    bool operator()(const std::pair<bid_t, value_t> &p1, const std::pair<bid_t, value_t> &p2)
    {
        return p1.second < p2.second;
    }
};

/** graph_scheduler
 *
 * This file contribute to define the interface how to schedule cache blocks
 */

class scheduler {
protected:
    metrics &_m;

public:
    scheduler(metrics& m) : _m(m) {  }

    virtual bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager) {
        return 0;
    }

    ~scheduler() {}
};

/**
 * The naive second-order scheduler which follows the graphwalker major constribution
 * In each schedule, load the block that has the most number of walks and processes them
 */
class navie_graphwalker_scheduler_t : public scheduler
{
private:
    std::vector<bid_t> buckets;
    bid_t exec_blk;
    size_t index;

    bid_t choose_block(graph_walk &walk_manager)
    {
        bid_t target_block = 0, nblocks = walk_manager.global_blocks->nblocks;
        wid_t max_walks = 0;
        for (bid_t cblk = 0; cblk < nblocks; ++cblk)
        {
            wid_t nwalks = 0;
            for (bid_t pblk = 0; pblk < nblocks; ++pblk)
                nwalks += walk_manager.nblockwalks(pblk * nblocks + cblk);
            if (nwalks > max_walks)
            {
                max_walks = nwalks;
                target_block = cblk;
            }
        }
        return target_block;
    }

    bid_t swap_block(graph_cache &cache, graph_walk &walk_manager, bid_t exclude_block)
    {
        bid_t blk = 0;
        int life = -1;
        for (bid_t p = 0; p < cache.ncblock; ++p)
        {
            if (cache.cache_blocks[p].block == NULL)
            {
                blk = p;
                break;
            }
            if (cache.cache_blocks[p].life > life && cache.cache_blocks[p].block->blk != exclude_block)
            {
                blk = p;
                life = cache.cache_blocks[p].life;
            }
        }
        if (cache.cache_blocks[blk].block != NULL)
        {
            cache.cache_blocks[blk].block->cache_index = walk_manager.global_blocks->nblocks;
        }
        return blk;
    }

    void swapin_block(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager, bid_t select_block, bid_t exclude_blk)
    {
        bid_t cache_index = (*(walk_manager.global_blocks))[select_block].cache_index;
        if (cache_index == walk_manager.global_blocks->nblocks)
        {
            cache_index = swap_block(cache, walk_manager, exclude_blk);
            driver.load_block_info(cache, walk_manager.global_blocks, cache_index, select_block);
        }
        cache.cache_blocks[cache_index].life = 0;
    }

public:
    navie_graphwalker_scheduler_t(metrics &m) : scheduler(m) {
        index = 0;
    }

    bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        bid_t nblocks = walk_manager.global_blocks->nblocks;
        _m.start_time("naive_graphwalker");
        while(index >= buckets.size()) {
            buckets.clear();
            exec_blk = choose_block(walk_manager);
            swapin_block(cache, driver, walk_manager, exec_blk, nblocks);
            for (bid_t p = 0; p < cache.ncblock; ++p)
                cache.cache_blocks[p].life++;
            for (bid_t blk = 0; blk < walk_manager.global_blocks->nblocks; blk++)
            {
                if (walk_manager.nblockwalks(blk * nblocks + exec_blk) > 0 || walk_manager.nblockwalks(exec_blk * nblocks + blk) > 0)
                {
                    buckets.push_back(blk);
                }
            }
            index = 0;
        }

        bid_t blk = buckets[index];
        swapin_block(cache, driver, walk_manager, blk, exec_blk);
        if (walk_manager.nblockwalks(blk * nblocks + exec_blk) > 0) {
            cache.walk_blocks.push_back(blk * nblocks + exec_blk);
        }

        if (blk != exec_blk && walk_manager.nblockwalks(exec_blk * nblocks + blk) > 0) {
            cache.walk_blocks.push_back(exec_blk * nblocks + blk);
        }

        _m.stop_time("naive_graphwalker");
        index++;

        return 0;
    }
};

/**
 * 
 */
class simulated_annealing_scheduler_t : public scheduler
{
private:
    std::vector<bid_t> bucket_sequences;
    std::vector<bid_t> buckets;
    size_t index, max_iter;
    
    int Combination(int n,int k) {
        if (k < 0 || k > n) {
            return 0;
        }
        int res = 1;
        for (int i = 0; i < k; ++i) {
            res *= (n - i);
            res /= (i + 1);
        }
        return res;
    }
    void choose_blocks(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        std::unordered_set<bid_t> cache_blocks;
        for(bid_t blk = 0; blk < cache.ncblock; blk++) {
            if(cache.cache_blocks[blk].block != NULL) cache_blocks.insert(cache.cache_blocks[blk].block->blk);
        }

        bid_t nblocks = walk_manager.nblocks;
        std::vector<wid_t> block_walks(nblocks * nblocks);
        for(bid_t blk = 0; blk < nblocks * nblocks; blk++) {
            block_walks[blk] = walk_manager.nblockwalks(blk);
        }

        std::vector<wid_t> partition_walks(nblocks, 0);
        for(bid_t c_blk = 0; c_blk < nblocks; c_blk++) {
            for (bid_t p_blk = 0; p_blk < nblocks; p_blk++) {
                partition_walks[c_blk] += block_walks[p_blk * nblocks + c_blk];
                // if(p_blk != c_blk) partition_walks[p_blk] += block_walks[c_blk * nblocks + p_blk];
            }
        }
        auto cmp = [&partition_walks, &walk_manager](bid_t u, bid_t v)
        {
            return partition_walks[u] > partition_walks[v];
        };
        std::vector<bid_t> block_indexs(nblocks, 0);
        std::iota(block_indexs.begin(), block_indexs.end(), 0);
        std::sort(block_indexs.begin(), block_indexs.end(), cmp);

        wid_t most_nwalks = 0;
        bid_t best_index = cache.ncblock - 1;
        for(bid_t p_index = cache.ncblock - 1; p_index < nblocks; p_index++) {
            wid_t nwalks = 0;
            for(bid_t c_index = 0; c_index < cache.ncblock - 1; c_index++) {
                nwalks += block_walks[block_indexs[p_index] * nblocks + block_indexs[c_index]] + block_walks[block_indexs[c_index] * nblocks + block_indexs[p_index]];
            }
            if(nwalks > most_nwalks) {
                best_index = p_index;
                most_nwalks = nwalks;
            }
        }
        std::swap(block_indexs[cache.ncblock - 1], block_indexs[best_index]);
        std::vector<bid_t> candidate_blocks(cache.ncblock);
        for (bid_t blk = 0; blk < cache.ncblock; blk++) candidate_blocks[blk] = block_indexs[blk];

        auto cal_score = [&block_walks, &walk_manager, nblocks](const std::vector<bid_t>& blocks) {
            wid_t score = 0;
            for(auto p_blk : blocks) {
                for(auto c_blk : blocks) {
                    score += block_walks[p_blk * nblocks + c_blk];
                }
            }
            return score;
        };

        size_t maxiter=Combination(walk_manager.nblocks,cache.ncblock);
        _m.start_time("act_simulated_annealing_scheduler_swap_blocks");
        if(cache.ncblock < nblocks) {
#ifdef TEMPERATURE_COOLING  
            real_t T = 100.0, alpha = 0.998;
            real_t endT=0.001;
#endif
            size_t iter = 0;
            size_t can_comm = 0;
            for(auto blk : candidate_blocks) if(cache_blocks.find(blk) != cache_blocks.end()) can_comm++;
            real_t y_can = cal_score(candidate_blocks) / (cache.ncblock - can_comm);

            std::srand(std::time(nullptr));
            while(iter<maxiter && T>endT) {
                std::vector<bid_t> tmp_blocks = candidate_blocks;
                size_t pos = rand() % (nblocks - cache.ncblock) + cache.ncblock, tmp_pos = rand() % cache.ncblock;
                std::swap(tmp_blocks[tmp_pos], block_indexs[pos]);
                size_t tmp_comm = 0;
                for(auto blk : tmp_blocks) if(cache_blocks.find(blk) != cache_blocks.end()) tmp_comm++;
                real_t y_tmp = 0.0;
                if(tmp_comm < cache.ncblock) y_tmp = cal_score(tmp_blocks) / (cache.ncblock - tmp_comm);

                if(y_tmp > y_can) {
                    candidate_blocks = tmp_blocks;
                    y_can = y_tmp;
                } else {
#ifdef TEMPERATURE_COOLING
                    real_t rand_val = static_cast<real_t>(std::rand()) / RAND_MAX;
                    real_t accept_prob = exp((y_tmp - y_can) / T);
                    if(y_tmp > 0 && rand_val < accept_prob) {
                        candidate_blocks = tmp_blocks;
                        y_can = y_tmp;
                    }else{
#endif
                        std::swap(tmp_blocks[tmp_pos], block_indexs[pos]);
#ifdef TEMPERATURE_COOLING
                    }
#endif
                }
#ifdef TEMPERATURE_COOLING
                T = alpha * T;  // cooling the temperature
#endif
                iter++;
            }
	        //std::cout<<"iter_count"<<iter<<std::endl;
        }
        _m.stop_time("act_simulated_annealing_scheduler_swap_blocks");
        buckets = candidate_blocks;
        std::unordered_set<bid_t> bucket_uncached, bucket_cached;

        for(bid_t blk = 0; blk < buckets.size(); blk++) {
            if(cache_blocks.find(buckets[blk]) != cache_blocks.end()) {
                bucket_cached.insert(buckets[blk]);
            }else{
                bucket_uncached.insert(buckets[blk]);
            }
        }

        size_t pos = 0;
        for(auto blk : bucket_cached) {
            bid_t cache_index = (*(walk_manager.global_blocks))[blk].cache_index;
            swap(cache.cache_blocks[pos], cache.cache_blocks[cache_index]);
            cache.cache_blocks[pos].block->cache_index = pos;
            cache.cache_blocks[cache_index].block->cache_index = cache_index;
            //std::cout << "swap block info, blk = " << blk << ", from " << cache_index << " to " << pos << std::endl;
            pos++;
        }

        for(auto blk : bucket_uncached) {
            if(partition_walks[blk] > 0) {
                if(cache.cache_blocks[pos].block != NULL) {
                    cache.cache_blocks[pos].block->cache_index = nblocks;
                }
                //std::cout << "load block info, blk = " << blk << " to " << pos << std::endl;
                driver.load_block_info(cache, walk_manager.global_blocks, pos, blk);
                pos++;
            }
        }

        std::cout << "bucket sequence : ";
        for(auto p_blk : buckets) {
            for(auto c_blk : buckets) {
                if(block_walks[p_blk * nblocks + c_blk] > 0) {
                    std::cout << p_blk << " -> " << c_blk << ", ";
                    cache.walk_blocks.push_back(p_blk * nblocks + c_blk);
                }
            }
        }
        std::cout << std::endl;
        if(cache.walk_blocks.empty()) {
            logstream(LOG_ERROR) << "random scheduler choose blocks without walks to update." << std::endl;
            exit(0);
        }
    }

public:
    simulated_annealing_scheduler_t(size_t param_max_iter, metrics &m) : scheduler(m)
    {
        max_iter = param_max_iter;
        index = 0;
    }

    bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        _m.start_time("simulated_annealing_scheduler_swap_blocks");
        choose_blocks(cache, driver, walk_manager);
        _m.stop_time("simulated_annealing_scheduler_swap_blocks");
        return 0;
    }
};

class lp_solver_scheduler_t : public scheduler
{
private:
    std::vector<bid_t> bucket_sequences;
    std::vector<bid_t> buckets;
    size_t index, max_iter;

    void choose_blocks(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        std::unordered_set<bid_t> cache_blocks;
        std::vector<bool> lp_cache_blocks(walk_manager.nblocks, false);
        size_t num_cache = 0;
        for (bid_t blk = 0; blk < cache.ncblock; blk++)
        {
            if (cache.cache_blocks[blk].block != NULL) {
                cache_blocks.insert(cache.cache_blocks[blk].block->blk);
                num_cache++;
                lp_cache_blocks[cache.cache_blocks[blk].block->blk] = true;
            }
        }

        bid_t nblocks = walk_manager.nblocks;
        std::vector<wid_t> block_walks(nblocks * nblocks);
        for (bid_t blk = 0; blk < nblocks * nblocks; blk++)
        {
            block_walks[blk] = walk_manager.nblockwalks(blk);
        }

        std::vector<wid_t> partition_walks(nblocks, 0);
        for (bid_t p_blk = 0; p_blk < nblocks; p_blk++)
        {
            for (bid_t c_blk = 0; c_blk < nblocks; c_blk++)
            {
                partition_walks[c_blk] += block_walks[p_blk * nblocks + c_blk];
                // if(p_blk != c_blk) partition_walks[c_blk] += block_walks[c_blk * nblocks + p_blk];
            }
        }

        std::vector<Edge_t> edges;
        for (bid_t blk = 0; blk < nblocks * nblocks; blk++)
        {
            if (block_walks[blk] > 0)
            {
                Edge_t edge = {blk / nblocks, blk % nblocks, block_walks[blk]};
                edges.push_back(edge);
            }
        }
        auto cal_score = [&block_walks, &walk_manager, nblocks](const std::vector<bid_t> &blocks)
        {
            wid_t score = 0;
            for (auto p_blk : blocks)
            {
                for (auto c_blk : blocks)
                {
                    score += block_walks[p_blk * nblocks + c_blk];
                }
            }
            return score;
        };
        std::vector<bid_t> candidate_blocks(cache.ncblock);
        std::vector<bid_t> block_indexs(nblocks, 0);
        std::iota(block_indexs.begin(), block_indexs.end(), 0);
        _m.start_time("lp_solve_schedule");
        bool success = false;
        double max_score = 0.0;
        for(size_t num = 1; num <= nblocks - cache.ncblock && num <=cache.ncblock; num++) {
            size_t num_select = (num_cache > 0) ? cache.ncblock - num : 0;
            DataModel data(edges, lp_cache_blocks, nblocks, edges.size(), cache.ncblock, num_select);
            std::vector<bool> ans_blocks(nblocks, false);
            bool ans = operations_research::lp_solve_schedule(data, ans_blocks);
            if(ans) {
                success = true;
                std::vector<bid_t> tmp_blocks(cache.ncblock);
                bid_t pos = 0;
                for(bid_t blk = 0; blk < nblocks; blk++) {
                    if(ans_blocks[blk]) {
                        tmp_blocks[pos++] = blk;
                    }
                }
                double tmp_score = cal_score(tmp_blocks);
                std::cout << "select " << num << ", score: " << tmp_score / num << std::endl;
                if(tmp_score / num > max_score) {
                    max_score = tmp_score / num;
                    candidate_blocks = tmp_blocks;
                }
            }
        }
        _m.stop_time("lp_solve_schedule");
        if (success)
        {
            logstream(LOG_DEBUG) << "using lp_solve_schedule to solve partition scheduling" << std::endl;
        }
        else
        {
            logstream(LOG_DEBUG) << "using greedy algorithm to solve partition scheduling" << std::endl;
            auto cmp = [&partition_walks, &walk_manager](bid_t u, bid_t v)
            {
                return partition_walks[u] > partition_walks[v];
            };
            std::sort(block_indexs.begin(), block_indexs.end(), cmp);

            wid_t most_nwalks = 0;
            bid_t best_index = cache.ncblock - 1;
            for (bid_t p_index = cache.ncblock - 1; p_index < nblocks; p_index++)
            {
                wid_t nwalks = 0;
                for (bid_t c_index = 0; c_index < cache.ncblock - 1; c_index++)
                {
                    nwalks += block_walks[block_indexs[p_index] * nblocks + block_indexs[c_index]] + block_walks[block_indexs[c_index] * nblocks + block_indexs[p_index]];
                }
                if (nwalks > most_nwalks)
                {
                    best_index = p_index;
                    most_nwalks = nwalks;
                }
            }
            std::swap(block_indexs[cache.ncblock - 1], block_indexs[best_index]);
            for (bid_t blk = 0; blk < cache.ncblock; blk++)
                candidate_blocks[blk] = block_indexs[blk];
        }

        buckets = candidate_blocks;
        std::unordered_set<bid_t> bucket_uncached, bucket_cached;

        for (bid_t blk = 0; blk < buckets.size(); blk++)
        {
            if (cache_blocks.find(buckets[blk]) != cache_blocks.end())
            {
                bucket_cached.insert(buckets[blk]);
            }
            else
            {
                bucket_uncached.insert(buckets[blk]);
            }
        }

        size_t pos = 0;
        for (auto blk : bucket_cached)
        {
            bid_t cache_index = (*(walk_manager.global_blocks))[blk].cache_index;
            swap(cache.cache_blocks[pos], cache.cache_blocks[cache_index]);
            cache.cache_blocks[pos].block->cache_index = pos;
            cache.cache_blocks[cache_index].block->cache_index = cache_index;
            std::cout << "swap block info, blk = " << blk << ", from " << cache_index << " to " << pos << std::endl;
            pos++;
        }

        for (auto blk : bucket_uncached)
        {
            if (partition_walks[blk] > 0)
            {
                if (cache.cache_blocks[pos].block != NULL)
                {
                    cache.cache_blocks[pos].block->cache_index = nblocks;
                }
                std::cout << "load block info, blk = " << blk << " -> cache_index = " << pos << std::endl;
                driver.load_block_info(cache, walk_manager.global_blocks, pos, blk);
                pos++;
            }
        }

        std::cout << "bucket sequence : ";
        for (auto p_blk : buckets)
        {
            for (auto c_blk : buckets)
            {
                if (block_walks[p_blk * nblocks + c_blk] > 0)
                {
                    std::cout << p_blk << " -> " << c_blk << ", ";
                    cache.walk_blocks.push_back(p_blk * nblocks + c_blk);
                }
            }
        }
        std::cout << std::endl;
    }

public:
    lp_solver_scheduler_t(metrics &m) : scheduler(m)
    {
        index = 0;
    }

    bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        _m.start_time("lp_solver_scheduler_swap_blocks");
        choose_blocks(cache, driver, walk_manager);
        _m.stop_time("lp_solver_scheduler_swap_blocks");
        return 0;
    }
};

class random_scheduler_t : public scheduler
{
private:
    void choose_blocks(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
    while(cache.walk_blocks.empty()){
        std::unordered_set<bid_t> cache_blocks;
        for (bid_t blk = 0; blk < cache.ncblock; blk++)
        {
            if (cache.cache_blocks[blk].block != NULL)
            {
                cache_blocks.insert(cache.cache_blocks[blk].block->blk);
            }
        }

        bid_t nblocks = walk_manager.nblocks;
        std::vector<wid_t> block_walks(nblocks * nblocks);
        for (bid_t blk = 0; blk < nblocks * nblocks; blk++)
        {
            block_walks[blk] = walk_manager.nblockwalks(blk);
        }

        std::vector<wid_t> partition_walks(nblocks, 0);
        for (bid_t p_blk = 0; p_blk < nblocks; p_blk++)
        {
            for (bid_t c_blk = 0; c_blk < nblocks; c_blk++)
            {
                partition_walks[c_blk] += block_walks[p_blk * nblocks + c_blk];
                if(p_blk != c_blk) partition_walks[c_blk] += block_walks[c_blk * nblocks + p_blk];
            }
        }

        std::vector<bid_t> remaining_blocks;
        for(bid_t blk = 0; blk < nblocks; blk++) {
            if(partition_walks[blk] > 0) remaining_blocks.push_back(blk);
        }

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(remaining_blocks.begin(), remaining_blocks.end(), std::default_random_engine(seed));

        std::vector<bid_t> candidate_blocks;
        for(bid_t blk_index = 0; blk_index < cache.ncblock && blk_index < remaining_blocks.size(); blk_index++) {
            candidate_blocks.push_back(remaining_blocks[blk_index]);
        }

        std::unordered_set<bid_t> bucket_uncached, bucket_cached;
        for (bid_t blk = 0; blk < candidate_blocks.size(); blk++)
        {
            if(cache_blocks.find(candidate_blocks[blk]) != cache_blocks.end()) {
                bucket_cached.insert(candidate_blocks[blk]);
            }
            else {
                bucket_uncached.insert(candidate_blocks[blk]);
            }
        }

        size_t pos = 0;
        for (auto blk : bucket_cached)
        {
            bid_t cache_index = (*(walk_manager.global_blocks))[blk].cache_index;
            swap(cache.cache_blocks[pos], cache.cache_blocks[cache_index]);
            cache.cache_blocks[pos].block->cache_index = pos;
            cache.cache_blocks[cache_index].block->cache_index = cache_index;
            std::cout << "swap block info, blk = " << blk << ", from " << cache_index << " to " << pos << std::endl;
            pos++;
        }

        for (auto blk : bucket_uncached)
        {
            if (partition_walks[blk] > 0)
            {
                if (cache.cache_blocks[pos].block != NULL)
                {
                    cache.cache_blocks[pos].block->cache_index = nblocks;
                }
                std::cout << "load block info, blk = " << blk << " -> cache_index = " << pos << std::endl;
                driver.load_block_info(cache, walk_manager.global_blocks, pos, blk);
                pos++;
            }
        }

        std::cout << "bucket sequence : " << candidate_blocks.size() << " ";
        for (auto p_blk : candidate_blocks)
        {
            for (auto c_blk : candidate_blocks)
            {
                if (block_walks[p_blk * nblocks + c_blk] > 0)
                {
                    std::cout << p_blk << " -> " << c_blk << ", ";
                    cache.walk_blocks.push_back(p_blk * nblocks + c_blk);
                }
            }
        }
        std::cout << std::endl;
        //if(cache.walk_blocks.empty()) {
        //    logstream(LOG_ERROR) << "random scheduler choose blocks without walks to update." << std::endl;
        //    exit(0);
        //}
        }
    }

public:
    random_scheduler_t(metrics &m) : scheduler(m) { }

    bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        _m.start_time("random_scheduler_swap_blocks");
        choose_blocks(cache, driver, walk_manager);
        _m.stop_time("random_scheduler_swap_blocks");
        return 0;
    }
};

/**
 * @brief the greedy scheduler choose `m` blocks which have the most walks
 *
 */
class greedy_graphwalker_scheduler_t : public scheduler
{
private:
    void choose_blocks(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        std::unordered_set<bid_t> cache_blocks;
        for (bid_t blk = 0; blk < cache.ncblock; blk++)
        {
            if (cache.cache_blocks[blk].block != NULL)
            {
                cache_blocks.insert(cache.cache_blocks[blk].block->blk);
            }
        }

        bid_t nblocks = walk_manager.nblocks;
        std::vector<wid_t> block_walks(nblocks * nblocks);
        for (bid_t blk = 0; blk < nblocks * nblocks; blk++)
        {
            block_walks[blk] = walk_manager.nblockwalks(blk);
        }

        std::vector<wid_t> from_p_walks(nblocks, 0), to_p_walks(nblocks, 0);
        for (bid_t p_blk = 0; p_blk < nblocks; p_blk++)
        {
            for (bid_t c_blk = 0; c_blk < nblocks; c_blk++)
            {
                to_p_walks[c_blk] += block_walks[p_blk * nblocks + c_blk];
                from_p_walks[p_blk] += block_walks[p_blk * nblocks + c_blk];
            }
        }

        std::vector<bid_t> remaining_blocks;
        for (bid_t blk = 0; blk < nblocks; blk++)
        {
            if (to_p_walks[blk] > 0 || from_p_walks[blk] > 0)
                remaining_blocks.push_back(blk);
        }

        auto cmp = [&to_p_walks, &walk_manager](bid_t u, bid_t v)
        {
            return to_p_walks[u] > to_p_walks[v];
        };

        std::sort(remaining_blocks.begin(), remaining_blocks.end(), cmp);

        std::vector<bid_t> candidate_blocks;
        for (bid_t blk_index = 0; blk_index < cache.ncblock && blk_index < remaining_blocks.size(); blk_index++)
        {
            candidate_blocks.push_back(remaining_blocks[blk_index]);
        }

        std::unordered_set<bid_t> bucket_uncached, bucket_cached;
        for (bid_t blk = 0; blk < candidate_blocks.size(); blk++)
        {
            if (cache_blocks.find(candidate_blocks[blk]) != cache_blocks.end())
            {
                bucket_cached.insert(candidate_blocks[blk]);
            }
            else
            {
                bucket_uncached.insert(candidate_blocks[blk]);
            }
        }

        size_t pos = 0;
        for (auto blk : bucket_cached)
        {
            bid_t cache_index = (*(walk_manager.global_blocks))[blk].cache_index;
            swap(cache.cache_blocks[pos], cache.cache_blocks[cache_index]);
            cache.cache_blocks[pos].block->cache_index = pos;
            cache.cache_blocks[cache_index].block->cache_index = cache_index;
            std::cout << "swap block info, blk = " << blk << ", from " << cache_index << " to " << pos << std::endl;
            pos++;
        }

        for (auto blk : bucket_uncached)
        {
            if (from_p_walks[blk] > 0 || to_p_walks[blk] > 0)
            {
                if (cache.cache_blocks[pos].block != NULL)
                {
                    cache.cache_blocks[pos].block->cache_index = nblocks;
                }
                std::cout << "load block info, blk = " << blk << " -> cache_index = " << pos << std::endl;
                driver.load_block_info(cache, walk_manager.global_blocks, pos, blk);
                pos++;
            }
        }

        std::cout << "bucket sequence : " << candidate_blocks.size() << " ";
        for (auto p_blk : candidate_blocks)
        {
            for (auto c_blk : candidate_blocks)
            {
                if (block_walks[p_blk * nblocks + c_blk] > 0)
                {
                    std::cout << p_blk << " -> " << c_blk << ", ";
                    cache.walk_blocks.push_back(p_blk * nblocks + c_blk);
                }
            }
        }
        std::cout << std::endl;
        while(cache.walk_blocks.empty()){       //随机选择
            std::cout<<"endless loop"<<std::endl;
            //metrics_report(_m);
            candidate_blocks.clear();
            bucket_cached.clear();
            bucket_uncached.clear();
            std::vector<wid_t> partition_walks(nblocks, 0);
            for (bid_t p_blk = 0; p_blk < nblocks; p_blk++)
            {
                for (bid_t c_blk = 0; c_blk < nblocks; c_blk++)
                {
                    partition_walks[c_blk] += block_walks[p_blk * nblocks + c_blk];
                    if(p_blk != c_blk) partition_walks[c_blk] += block_walks[c_blk * nblocks + p_blk];
                }
            }
            std::vector<bid_t> remaining_blocks;
            for(bid_t blk = 0; blk < nblocks; blk++) {
                if(partition_walks[blk] > 0) remaining_blocks.push_back(blk);
            }

            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(remaining_blocks.begin(), remaining_blocks.end(), std::default_random_engine(seed));
            for(bid_t blk_index = 0; blk_index < cache.ncblock && blk_index < remaining_blocks.size(); blk_index++) {
                candidate_blocks.push_back(remaining_blocks[blk_index]);
            }

            for (bid_t blk = 0; blk < candidate_blocks.size(); blk++)
            {
                if(cache_blocks.find(candidate_blocks[blk]) != cache_blocks.end()) {
                    bucket_cached.insert(candidate_blocks[blk]);
                }
            else {
                bucket_uncached.insert(candidate_blocks[blk]);
            }
        }

            size_t pos = 0;
            for (auto blk : bucket_cached)
            {
            bid_t cache_index = (*(walk_manager.global_blocks))[blk].cache_index;
            swap(cache.cache_blocks[pos], cache.cache_blocks[cache_index]);
            cache.cache_blocks[pos].block->cache_index = pos;
            cache.cache_blocks[cache_index].block->cache_index = cache_index;
            std::cout << "swap block info, blk = " << blk << ", from " << cache_index << " to " << pos << std::endl;
            pos++;
            }

        for (auto blk : bucket_uncached)
        {
            if (partition_walks[blk] > 0)
            {
                if (cache.cache_blocks[pos].block != NULL)
                {
                    cache.cache_blocks[pos].block->cache_index = nblocks;
                }
                std::cout << "load block info, blk = " << blk << " -> cache_index = " << pos << std::endl;
                driver.load_block_info(cache, walk_manager.global_blocks, pos, blk);
                pos++;
            }
        }

        std::cout << "bucket sequence : " << candidate_blocks.size() << " ";
        for (auto p_blk : candidate_blocks)
        {
            for (auto c_blk : candidate_blocks)
            {
                if (block_walks[p_blk * nblocks + c_blk] > 0)
                {
                    std::cout << p_blk << " -> " << c_blk << ", ";
                    cache.walk_blocks.push_back(p_blk * nblocks + c_blk);
                }
            }
        }
        std::cout << std::endl;
            //metrics_report(_m);
            //randomchoice(cache, driver, walk_manager);
        }
    
    }

public:
    greedy_graphwalker_scheduler_t(metrics &m) : scheduler(m) {}

    bid_t schedule(graph_cache &cache, graph_driver &driver, graph_walk &walk_manager)
    {
        _m.start_time("greedy_graphwalker_scheduler_swap_blocks");
        choose_blocks(cache, driver, walk_manager);
        _m.stop_time("greedy_graphwalker_scheduler_swap_blocks");
        return 0;
    }
};

#endif
