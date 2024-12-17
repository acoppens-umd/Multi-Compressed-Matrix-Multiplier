#ifndef processing_element_multiplier_h
#define processing_element_multiplier_h

#include "kernel.h"
#include "types.h"

void processing_element_multiplier(
    const int a_num_rows,
    //in
    hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> &a_data_stream, //from read_controller 
    hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> &b_data_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream, //from PE
    
    //out
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> &out_stream
);

#endif