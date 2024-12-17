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
    a_in_port_addr_t start_indptr;
    a_in_port_addr_t end_indptr;
    bool end;
};

typedef ap_uint<B_IN_WIDTH> b_in_port_t;
typedef ap_uint<B_MEMORY_SPACE_ADDR_WIDTH> b_in_port_addr_t;
struct BInPortReadReq_t {
    b_in_port_addr_t start_indptr;
    b_in_port_addr_t end_indptr;
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
    ap_uint<32> row;
    ap_uint<32> start_indptr;
    ap_uint<32> end_indptr;
    bool end;
};

//typedef ap_uint<B_DENSE_OBJECT_DATA_WIDTH> task_b_position_t;
struct TaskBPosition_t {
    ap_uint<32> col;
    ap_uint<32> start_indptr;
    ap_uint<32> end_indptr;
    bool end;
};

struct DenseVal_t {
    float val;           // Tag size based on CACHE_SIZE
    ap_uint<32> dest;
    bool valid;
};

struct AtomicInstruction_t{
    ap_uint<32> row;
    ap_uint<32> col;
    ap_uint<32> a_start_indptr;
    ap_uint<32> a_end_indptr;
    ap_uint<32> b_start_indptr;
    ap_uint<32> b_end_indptr;
    bool end;
};

#endif