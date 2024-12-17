#ifndef write_back_network_h
#define write_back_network_h

#include "kernel.h"
#include "types.h"

#define BITONIC false

void write_back_network(hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
                        hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES],
                        ap_uint<32> *out_data, int &write_back_counter, int &atomic_write_counter);

inline void read_in(hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
                    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES],
                    DenseVal_t in_array[NUM_PES], bool &active, int &write_back_counter);

inline void swap_and_merge(DenseVal_t &in_val1, DenseVal_t &in_val2, 
                           DenseVal_t &out_val1, DenseVal_t &out_val2);


#if BITONIC 
void bitonic_sort(DenseVal_t (&in_array)[NUM_PES], DenseVal_t (&out_array)[NUM_PES]);
#endif

inline void write_back(DenseVal_t (&out_array)[NUM_PES], ap_uint<32> *out_data,
                       int &atomic_write_counter);

#endif 