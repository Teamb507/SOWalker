#ifndef _GRAPH_TYPES_H_
#define _GRAPH_TYPES_H_

#include <stdint.h>
#include <functional>
#include <vector>
#include <climits>

typedef uint32_t vid_t;   /* vertex id */
typedef uint64_t eid_t;   /* edge id */
typedef uint32_t bid_t;   /* block id */
typedef uint32_t rank_t;  /* block rank */
typedef uint16_t hid_t;   /* walk hop */
typedef uint16_t tid_t;   /* thread id */
typedef uint32_t wid_t;   /* walk id */
typedef uint64_t walk_t;  /* walker data type */
typedef float    real_t;     /* edge weight */

#define HOP_SIZE 8           /* hop field size */
#define VERTEX_SIZE 28      /* source, previous, current vertex size */
#define WALKER_ID_SIZE 36   /* walker id size */
enum WeightType { UNWEIGHTED, WEIGHTED };
struct walker_t {
    wid_t id;
    vid_t source, previous, current;
    bid_t hop;
    bid_t cur_index, prev_index;
};

walker_t walker_makeup(wid_t id, vid_t source, vid_t previous, vid_t pos, hid_t hop, bid_t c_index, bid_t p_index)
{
    walker_t walk_data;
    walk_data.id = id;
    walk_data.source = source;
    walk_data.previous = previous;
    walk_data.current = pos;
    walk_data.hop = hop;
    walk_data.cur_index = c_index;
    walk_data.prev_index = p_index;
    return walk_data;
}

#define WALKER_ID(walk) (walk.id)
#define WALKER_SOURCE(walk) (walk.source)
#define WALKER_PREVIOUS(walk) (walk.previous)
#define WALKER_POS(walk) (walk.current)
#define WALKER_HOP(walk) (walk.hop)
#define WALKER_CUR_BLOCK(walk) (walk.cur_index)
#define WALKER_PREV_BLOCK(walk) (walk.prev_index)


#endif
