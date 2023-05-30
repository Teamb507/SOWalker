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
    //size_t blocksize = get_option_long("blocksize", BLOCK_SIZE);
    size_t blocksize = get_option_int("blocksize", BLOCK_SIZE);
    size_t nthreads = get_option_int("nthreads", omp_get_max_threads());
    size_t dynamic   = get_option_bool("dynamic");   // the blocksize is dynamic, according to the number of walks
    //size_t cache_size = get_option_int("cache", MEMORY_CACHE / (1024LL * 1024 * 1024));
    size_t cache_size = get_option_int("cache", MEMORY_CACHE / (1024 * 1024));
    size_t max_iter = get_option_int("iter", 30);
    wid_t walks = (wid_t)get_option_int("walkpersource", 1);
    hid_t steps = (hid_t)get_option_int("length", 20);
    real_t p = (real_t)get_option_float("p", 0.5); 
    real_t q = (real_t)get_option_float("q", 2.0); 
    std::string sort_method=get_option_string("sort","default");
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
        //cache_size * 1024LL * 1024 * 1024,
        cache_size * 1024 * 1024,
        blocksize,
        (tid_t)nthreads,
        (tid_t)omp_get_max_threads(),
        nvertices,
        nedges,
        minimum_id,
        weighted
    };
    //BFS_sort sorter(&conf);
    //sorter.sort_graph();
    if (sort_method=="BFS"){
        BFS_sort sorter(&conf);
        sorter.sort_graph();
    }
    else if(sort_method=="BBFS"){
        Bounded_BFS_sort sorter(&conf,2);
        sorter.sort_graph();
    }
    else if(sort_method=="Random"){
        Random_sort sorter(&conf);
        sorter.sort_graph();
    }
    else if(sort_method=="Default"){
        
    }

    return 0;
}
