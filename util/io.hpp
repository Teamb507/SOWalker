#ifndef _GRAPH_IO_H_
#define _GRAPH_IO_H_

#include <vector>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include "api/types.hpp"
#include "util/util.hpp"
#include "logger/logger.hpp"

template<typename T>
std::vector<T> load_graph_blocks(std::string name) {
    std::ifstream isfile(name.c_str(), std::ios::in | std::ios::binary);
    assert(isfile);

    // get length of file
    isfile.seekg(0, isfile.end);
    bid_t nblocks = isfile.tellg() / sizeof(T);
    isfile.seekg(0, isfile.beg);

    std::vector<T> blocks(nblocks);
    isfile.read((char*)&blocks[0], nblocks * sizeof(T));
    isfile.close();

    return blocks;
}

void load_graph_meta(std::string base_name, vid_t *nvertices, eid_t *nedges, vid_t *min_vert,bool is_weighted) {
    if(is_weighted) {
        //TODO; this place exists some bug
        std::string edge_weight_path = get_weights_name(base_name);
        bool is_weight_exist = test_exists(edge_weight_path);
        ASSERT(is_weight_exist, "If the graph is weighted, then preprocessed weighted file must exist.");
    }
    std::string metafile = get_meta_name(base_name);
    auto metastream = std::fstream(metafile.c_str(), std::ios::in | std::ios::binary);
    metastream.read((char*)nvertices, sizeof(vid_t));
    metastream.read((char*)nedges, sizeof(eid_t));
    metastream.read((char*)min_vert, sizeof(vid_t));
    metastream.close();
}

template<typename T>
void load_block_range(int fd, T *buf, size_t count, off_t off) {   //off代表偏移量
    size_t nbr = 0;  /* number of bytes has read */
    size_t total = sizeof(T) * count; /* the bytes that need to read */
    char* bufptr = (char *)buf;
    while(nbr < total) {
        size_t ret = pread(fd, bufptr, total - nbr, off);
        assert(ret > 0);
        bufptr += ret;
        nbr += ret;
        off += ret;
    }
}

template<typename T>
void dump_block_range(int fd, T *buf, size_t count, off_t off) {
    size_t nbw = 0; /* number of bytes has written */
    size_t total = sizeof(T) * count;
    char *bufptr = (char*) buf;
    while(nbw < total) {
        size_t ret = pwrite(fd, bufptr, total - nbw, off);
        assert(ret > 0);
        bufptr += ret;
        nbw += ret;
        off += ret;
    }
}

template<typename T>
void appendfile(std::string filename, T *array, size_t bsize) {
    int fd = open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
    assert(fd >= 0);
    // pwrite(fd, (char*)array, bsize * sizeof(T), 0);
    dump_block_range(fd, array, bsize, 0);
    close(fd);
}

#endif
