#ifndef cache_h
#define cache_h

#include "kernel.h"
#include "types.h"

#define A_CACHE_ADDR_WIDTH 12
#define A_TAG_WIDTH (A_MEMORY_SPACE_ADDR_WIDTH - A_CACHE_ADDR_WIDTH)
#define B_CACHE_ADDR_WIDTH 12
#define B_TAG_WIDTH (B_MEMORY_SPACE_ADDR_WIDTH - B_CACHE_ADDR_WIDTH)

struct ACacheLine {
    bool valid;
    ap_uint<A_TAG_WIDTH> tag;           // Tag size based on CACHE_SIZE
};

struct BCacheLine {
    bool valid;
    ap_uint<B_TAG_WIDTH> tag;
};

#endif