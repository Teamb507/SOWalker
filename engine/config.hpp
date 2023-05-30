#ifndef _GRAPH_CONFIG_H_
#define _GRAPH_CONFIG_H_

#include <string>
#include "api/types.hpp"

/** config
 *
 * This file contribute to define the graph config structure
 */

struct graph_config {
    std::string base_name;
    size_t cache_size;
    size_t blocksize;
    tid_t nthreads;
    tid_t max_nthreads;

    vid_t nvertices;
    eid_t nedges;
    vid_t min_vert;
    bool is_weighted;
};

#endif
