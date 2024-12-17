#ifndef processing_element_h
#define processing_element_h

#include "kernel.h"
#include "types.h"
#include "cache.h"

enum pe_computation_mode {
    DENSE_BLOCK_DENSE_BLOCK = 0,
    DENSE_DIAG_DENSE_BLOCK = 1,
    DENSE_BLOCK_DENSE_DIAG = 2,
    DENSE_DIAG_DENSE_DIAG = 3,
    FINISH = 7
};

inline void cache_aware_send(ACacheLine (&a_cache_memory)[A_CACHE_SIZE], 
                             BCacheLine (&b_cache_memory)[B_CACHE_SIZE],
                             ap_uint<32> &dest_i, ap_uint<32> &dest_k,
                             a_in_port_addr_t &a_addr, b_in_port_addr_t &b_addr,
                             ap_uint<2> &a_offset, ap_uint<2> &b_offset,
                             hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
                             hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,
                             hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream);

void processing_element(
    const int pe,
    //in
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> &task_a_position_stream, //from task distributor
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> &task_b_position_stream,
    
    ap_uint<32> a_rows,
    ap_uint<32> a_cols,
    ap_uint<32> b_rows,
    ap_uint<32> b_cols,
    
    //out
    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &out_stream
);



#endif