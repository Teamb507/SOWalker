#ifndef _GRAPH_SPLIT_H_
#define _GRAPH_SPLIT_H_

#include <string>
#include <vector>
#include <fstream>
#include "api/graph_buffer.hpp"
#include "api/constants.hpp"
#include "api/types.hpp"
#include "logger/logger.hpp"
#include "util/util.hpp"
#include "util/io.hpp"

/* split the beg_pos into multiple blocks, each block max size is BLOCKSIZE */
size_t split_blocks(const std::string& base_name, int fnum, size_t block_size) {
    eid_t max_nedges = (eid_t)block_size / sizeof(vid_t);
    logstream(LOG_INFO) << "start split blocks, blocksize = " << block_size / (1024 * 1024) << "MB, max_nedges = " << max_nedges << std::endl;

    vid_t cur_pos  = 0;
    eid_t rd_edges = 0;  /* read edges */
    vid_t rd_verts = 0;  /* read vertices */
    std::vector<vid_t> vblocks;  /* vertex blocks */
    std::vector<eid_t> eblocks;  /* edge   blocks */
    vblocks.push_back(cur_pos);
    eblocks.push_back(rd_edges);

    std::string name = get_beg_pos_name(base_name);
    int fd = open(name.c_str(), O_RDONLY);
    assert(fd >= 0);
    vid_t nvertices = lseek(fd, 0, SEEK_END) / sizeof(eid_t);  
    logstream(LOG_INFO) << "split blocks, nvertics = " << nvertices-1<< std::endl;
    eid_t *beg_pos = (eid_t*)malloc(VERT_SIZE * sizeof(eid_t));
    assert(beg_pos != NULL);

    vid_t rv;
    while(rd_verts < nvertices) {
        rv = min_value(nvertices - rd_verts, VERT_SIZE);
        load_block_range(fd, beg_pos, (size_t)rv, (off_t)rd_verts * sizeof(eid_t));
        for(vid_t v = 0; v < rv; v++) {
            if(beg_pos[v] - rd_edges > max_nedges) {
                logstream(LOG_INFO) << "Block " << vblocks.size() - 1 << " : [ " << cur_pos << ", " << rd_verts + v - 1 << " ), csr position : [ " << rd_edges << ", " << beg_pos[v-1] << " )" << std::endl;
                cur_pos = rd_verts + v - 1;
                vblocks.push_back(cur_pos);
                rd_edges = beg_pos[v-1];
                eblocks.push_back(rd_edges);
            }
        }
        rd_verts += rv;
    }

    logstream(LOG_INFO) << "Block " << vblocks.size() - 1 << " : [ " << cur_pos << ", " << nvertices - 1 << " ), csr position : [ " << rd_edges << ", " << beg_pos[rv-1] << " )" << std::endl;
    logstream(LOG_INFO) << "Total blocks num : " << vblocks.size() << std::endl;
    close(fd);
    vblocks.push_back(nvertices-1);
    rd_edges = beg_pos[rv-1];
    eblocks.push_back(rd_edges);

    /** write the vertex split points into vertex block file */
    std::string vblockfile = get_vert_blocks_name(base_name, block_size);
    auto vblf = std::fstream(vblockfile.c_str(), std::ios::out | std::ios::binary);
    vblf.write((char*)&vblocks[0], vblocks.size() * sizeof(vid_t));
    vblf.close();

    /** write the edge split points into edge block file */
    std::string eblockfile = get_edge_blocks_name(base_name, block_size);
    auto eblf = std::fstream(eblockfile.c_str(), std::ios::out | std::ios::binary);
    eblf.write((char*)&eblocks[0], eblocks.size() * sizeof(eid_t));
    eblf.close();

    return vblocks.size() - 1;
}

#endif
