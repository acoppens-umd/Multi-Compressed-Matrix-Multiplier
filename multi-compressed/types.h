#ifndef types_h
#define types_h

#include "const.h"

#include <ap_int.h>

typedef union {
    int as_uint32;
    float as_float32;
} bit_float;

typedef ap_uint<A_IN_WIDTH> a_in_port_t;
typedef ap_uint<A_MEMORY_SPACE_ADDR_WIDTH> a_in_port_addr_t;
struct AInPortReadReq_t {
    a_in_port_addr_t addr;
    bool end;
};

typedef ap_uint<B_IN_WIDTH> b_in_port_t;
typedef ap_uint<B_MEMORY_SPACE_ADDR_WIDTH> b_in_port_addr_t;
struct BInPortReadReq_t {
    b_in_port_addr_t addr;
    bool end;
};

typedef ap_uint<A_META_IN_WIDTH> a_meta_in_port_t;
typedef ap_uint<A_META_MEMORY_SPACE_ADDR_WIDTH> a_meta_in_port_addr_t;
typedef ap_uint<B_META_IN_WIDTH> b_meta_in_port_t;
typedef ap_uint<B_META_MEMORY_SPACE_ADDR_WIDTH> b_meta_in_port_addr_t;

typedef ap_uint<OUT_WIDTH> out_port_t;
typedef ap_uint<OUT_MEMORY_SPACE_ADDR_WIDTH> out_port_addr_t;

//typedef ap_uint<A_DENSE_OBJECT_DATA_WIDTH> task_a_position_t;
struct TaskAPosition_t {
    a_in_port_addr_t addr;
    ap_uint<32> start_i;
    ap_uint<32> start_j;
    ap_uint<32> end_i;
    ap_uint<32> end_j;
    bool end;
};

//typedef ap_uint<B_DENSE_OBJECT_DATA_WIDTH> task_b_position_t;
struct TaskBPosition_t {
    b_in_port_addr_t addr;
    ap_uint<32> start_j;
    ap_uint<32> start_k;
    ap_uint<32> end_j;
    ap_uint<32> end_k;
    bool end;
};

struct DenseVal_t {
    float val;           // Tag size based on CACHE_SIZE
    ap_uint<32> dest_i;
    ap_uint<32> dest_k;
    bool valid;
};

struct AtomicInstruction_t{
    ap_uint<32> dest_i;
    ap_uint<32> dest_k;
    a_in_port_addr_t a_addr;
    b_in_port_addr_t b_addr;
    ap_uint<2> a_offset;
    ap_uint<2> b_offset;
    bool use_cached_a;
    bool use_cached_b;
    bool end;
};

#endif