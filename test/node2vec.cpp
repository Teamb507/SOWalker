#include <omp.h>
#include <functional>
#include "api/constants.hpp"
#include "engine/config.hpp"
#include "engine/cache.hpp"
#include "engine/schedule.hpp"
#include "engine/walk.hpp"
#include "engine/engine.hpp"
#include "logger/logger.hpp"
#include "util/io.hpp"
#include "util/util.hpp"
#include "metrics/metrics.hpp"
#include "metrics/reporter.hpp"
#include "apps/secondorder.hpp"
#include "preprocess/graph_converter.hpp"
#include "preprocess/graph_sort.hpp"
#include <iostream>
#include <fstream>

int main(int argc, const char *argv[])
{
    assert(argc >= 2);
    set_argc(argc, argv);
    logstream(LOG_INFO) << "app : " << argv[0] << ", dataset : " << argv[1] << std::endl;

    std::string input = argv[1];
    bool weighted = get_option_bool("weighted");
    bool sorted   = get_option_bool("sorted");
    bool skip     = get_option_bool("skip");        // use to skip the interactive convert query
    bool  c_update = get_option_bool("c_update");   
    size_t blocksize = get_option_long("blocksize", BLOCK_SIZE);
    size_t memory_size = get_option_int("memory", MEMORY_CACHE / (1024LL * 1024 * 1024));
    size_t nthreads = get_option_int("nthreads", omp_get_max_threads());
    size_t dynamic   = get_option_bool("dynamic");   // the blocksize is dynamic, according to the number of walks
    size_t max_iter = get_option_int("iter", 30);
    wid_t walks = (wid_t)get_option_int("walkpersource", 1);
    hid_t steps = (hid_t)get_option_int("length", 30);
    real_t p = (real_t)get_option_float("p", 0.5); 
    real_t q = (real_t)get_option_float("q", 2.0); 
    std::string schedule=get_option_string("schedule","sa");
    bool help_info = get_option_bool("h");

    if(help_info){
        std::cout<<"- dataset:       the dataset path"<<std::endl;
        std::cout<<"- weighted:      whether the dataset is weighted"<<std::endl;
        std::cout<<"- sorted:        whether the vertex neighbors is sorted"<<std::endl;
        std::cout<<"- skip:          whether to skip preprocessing"<<std::endl;
        std::cout<<"- c_update       whether to update walks continuously"<<std::endl;
        std::cout<<"- blocksize:     the size of each block"<<std::endl;
        std::cout<<"- nthreads:      the number of threads to walk"<<std::endl;
        std::cout<<"- dynamic:       whether the blocksize is dynamic, according to the number of walks"<<std::endl;
        std::cout<<"- memory_size:   the size(GB) of memory"<<std::endl;
        std::cout<<"- max_iter:      the maximum number of iteration for simulated annealing scheduler"<<std::endl;
        std::cout<<"- walkpersource: the number of walks for each vertex"<<std::endl;
        std::cout<<"- length:        the number of step for each walk"<<std::endl;
        std::cout<<"- p:             node2vec parameter"<<std::endl;
        std::cout<<"- q:             node2vec parameter"<<std::endl;
        std::cout<<"- schedule:      scheduling strategy"<<std::endl;
        std::cout<<"- h:             print this message"<<std::endl;
        return 0;
    }  
    metrics m("node2vec_walkpersource_" + std::to_string(walks) + "_steps_" + std::to_string(steps) + "_dataset_" + argv[1] );
    auto static_query_blocksize = [blocksize](vid_t nvertices) { return blocksize; };
    auto dynamic_query_blocksize = [&blocksize, walks](vid_t nvertices) {
        wid_t nwalks = 1LL * walks * nvertices;
        int dom = 0;
        while(nwalks){
            dom++;
            nwalks /= 10;
        }
        blocksize= 1LL * pow(2, 11 + dom) * 1024;
        logstream(LOG_INFO) << "determined blocksize = " << blocksize / (1024 * 1024) << "MB." << std::endl;
        return blocksize;
    };
    std::function<size_t(vid_t nvertices)> query_blocksize;
    if(dynamic) query_blocksize = dynamic_query_blocksize;
    else query_blocksize = static_query_blocksize;

    graph_converter converter(remove_extension(argv[1]), weighted, sorted);
    convert(input, converter, query_blocksize, skip);
    std::string base_name = remove_extension(argv[1]);

    /* graph meta info */
    vid_t nvertices;
    eid_t nedges;
    vid_t minimum_id;
    load_graph_meta(base_name, &nvertices, &nedges,&minimum_id,weighted);

    graph_config conf = {
        base_name,
        memory_size * 1024LL * 1024 * 1024,
        blocksize,
        (tid_t)nthreads,
        (tid_t)omp_get_max_threads(),
        nvertices,
        nedges,
        minimum_id,
        weighted
    };
        
    graph_block blocks(&conf);
    graph_driver driver(&conf, m);
    graph_walk walk_mangager(conf, driver, blocks);
    bid_t nmblocks = get_option_int("nmblocks", blocks.nblocks);
    graph_cache cache(min_value(nmblocks, blocks.nblocks), &conf);
    m.set("nblocks", std::to_string(blocks.nblocks));
    m.set("ncblocks", std::to_string(cache.ncblock));

    node2vec_app_t userprogram(walks, steps,c_update, p, q);
    graph_engine engine(cache, walk_mangager, driver, conf, m);
    scheduler* walk_scheduler;
    if(schedule=="lp"){
        walk_scheduler=new lp_solver_scheduler_t(m);
    }
    else if(schedule=="gw"){
        walk_scheduler=new navie_graphwalker_scheduler_t(m);    
    }
    else if(schedule=="random"){
        walk_scheduler=new random_scheduler_t(m);
    }
    else if(schedule=="greedy"){
        walk_scheduler=new greedy_graphwalker_scheduler_t(m);
    }
    else{
        walk_scheduler=new simulated_annealing_scheduler_t(max_iter, m);
    }
    m.set("scheduler", schedule);
    m.set("c_update", c_update);
    auto init_func = [walks](graph_walk *walk_manager)
    {
        #pragma omp parallel for schedule(static)
        for (vid_t vertex = 0; vertex < walk_manager->nvertices; vertex++)
        {
            wid_t idx = vertex * walks;
            for(wid_t off = 0; off < walks; off++) {
                bid_t index = walk_manager->global_blocks->get_block(vertex);
                walker_t walker = walker_makeup(idx + off, vertex, vertex, vertex, 0, index, index);
                walk_manager->move_walk(walker);
            }
        }
    };
    engine.prologue(userprogram, init_func);
    engine.run(userprogram, walk_scheduler);
    engine.epilogue(userprogram);
#ifdef PROF_METRIC
    blocks.report();
#endif
    metrics_report(m);
    return 0;
}
