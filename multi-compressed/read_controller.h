#ifndef read_controller_h
#define read_controller_h

#include "kernel.h"
#include "types.h"

void read_controller(const a_in_port_t *a_in_port,
                     const b_in_port_t *b_in_port,
                     hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&a_position_stream)[NUM_PES],
                     hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&b_position_stream)[NUM_PES],
                     hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> (&a_data_stream)[NUM_PES],
                     hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> (&b_data_stream)[NUM_PES]);

inline void serve_a_reads(const a_in_port_t *a_in_port,
                          hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&a_position_stream)[NUM_PES],
                          hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> (&a_data_stream)[NUM_PES]);

inline void serve_b_reads(const b_in_port_t *b_in_port,
                          hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&b_position_stream)[NUM_PES],
                          hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> (&b_data_stream)[NUM_PES]);

#endif