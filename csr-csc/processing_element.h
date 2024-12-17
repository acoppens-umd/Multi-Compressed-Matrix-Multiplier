#ifndef processing_element_h
#define processing_element_h

#include "kernel.h"
#include "types.h"

void processing_element(
    //in
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> &task_a_position_stream, //from job distributor
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> &task_b_position_stream,
    
    //out
    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream
);



#endif