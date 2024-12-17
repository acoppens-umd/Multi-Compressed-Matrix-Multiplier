#ifndef write_back_gate_h
#define write_back_gate_h

#include "kernel.h"
#include "types.h"

void write_back_gate(hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&in_streams)[NUM_PES],
                     hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
                     hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES]);

#endif