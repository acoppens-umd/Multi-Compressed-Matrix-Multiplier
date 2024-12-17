#include "write_back_network.h"

void write_back_network(
    hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES],

    //out
    ap_uint<32> *out_data,

    int &write_back_counter,
    int &atomic_write_counter
) {

    static bool active = true;
    static int write_back_counter_static = 0;
    static int atomic_write_counter_static = 0;

    write_back_pipeline:
    while (active) {
        #pragma HLS PIPELINE II=1

#if BITONIC 
        DenseVal_t in_array[NUM_PES];
        #pragma HLS array_partition variable=in_array type=complete
        DenseVal_t out_array[NUM_PES];
        #pragma HLS array_partition variable=out_array type=complete

        read_in(gate_to_network_initiation_stream, gate_to_network_streams, in_array, 
                active, write_back_counter_static);
        
        bitonic_sort(in_array, out_array);

        write_back(a_num_rows, b_num_cols, out_array, out_data, atomic_write_counter_static);
#else
        DenseVal_t out_array[NUM_PES];
        #pragma HLS array_partition variable=out_array type=complete

        read_in(gate_to_network_initiation_stream, gate_to_network_streams, out_array, 
                active, write_back_counter_static);

        write_back(out_array, out_data, atomic_write_counter_static);
#endif
    }

    write_back_counter = write_back_counter_static;
    atomic_write_counter = atomic_write_counter_static;
}

//Get writes from PEs
inline void read_in(hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
                    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES],
                    DenseVal_t in_array[NUM_PES], bool &active, int &write_back_counter) {
    bool gate_message = gate_to_network_initiation_stream.read();
    active = gate_message;
    write_back_counter++;
    
    read_into_pipeline_loop:
    for (int pe = 0; pe < NUM_PES; pe++) {
        #pragma HLS UNROLL

        in_array[pe] = gate_to_network_streams[pe].read();
    }
}

inline void swap_and_merge(DenseVal_t &in_val1, DenseVal_t &in_val2, 
                           DenseVal_t &out_val1, DenseVal_t &out_val2) {
    //same dest
    if (in_val1.dest == in_val2.dest) {
        out_val1.val = in_val1.val + in_val2.val;
        out_val1.dest = in_val1.dest;
        out_val1.valid = in_val1.valid || in_val2.valid;

        out_val2.val = 0.0f;
        out_val2.dest = in_val2.dest;
        out_val2.valid = false;

    //in_val1 first
    } else if (in_val1.dest < in_val2.dest) {
        out_val1.val = in_val1.val;
        out_val1.dest = in_val1.dest;
        out_val1.valid = in_val1.valid;

        out_val2.val = in_val2.val;
        out_val2.dest = in_val2.dest;
        out_val2.valid = in_val2.valid;
    
    //in_val2 first
    } else {
        out_val1.val = in_val2.val;
        out_val1.dest = in_val2.dest;
        out_val1.valid = in_val2.valid;

        out_val2.val = in_val1.val;
        out_val2.dest = in_val1.dest;
        out_val2.valid = in_val1.valid;
    }
}

#if BITONIC
void bitonic_sort(DenseVal_t (&in_array)[NUM_PES], DenseVal_t (&out_array)[NUM_PES]) {
#pragma HLS PIPELINE 

    DenseVal_t layer0[NUM_PES];
    #pragma HLS array_partition variable=layer0 type=complete

    pipeline_layer0:
    for (int pe = 0; pe < NUM_PES; pe += 2) {
        #pragma HLS UNROLL
        swap_and_merge(in_array[pe], in_array[pe + 1], layer0[pe], layer0[pe + 1]);
    }

    DenseVal_t layer1[NUM_PES];
    #pragma HLS array_partition variable=layer1 type=complete
    
    pipeline_layer1:
    for (int pe = 0; pe < NUM_PES; pe += 4) {
        #pragma HLS UNROLL
        swap_and_merge(layer0[pe], layer0[pe + 3], layer1[pe], layer1[pe + 3]);
        swap_and_merge(layer0[pe + 1], layer0[pe + 2], layer1[pe + 1], layer1[pe + 2]);
    }

    DenseVal_t layer2[NUM_PES];
    #pragma HLS array_partition variable=layer2 type=complete
    
    pipeline_layer2:
    for (int pe = 0; pe < NUM_PES; pe += 2) {
        #pragma HLS UNROLL
        swap_and_merge(layer1[pe], layer1[pe + 1], layer2[pe], layer2[pe + 1]);
    }

    DenseVal_t layer3[NUM_PES];
    #pragma HLS array_partition variable=layer3 type=complete
    
    pipeline_layer3:
    for (int pe = 0; pe < NUM_PES; pe += 8) {
        #pragma HLS UNROLL
        swap_and_merge(layer2[pe], layer2[pe + 7], layer3[pe], layer3[pe + 7]);
        swap_and_merge(layer2[pe + 1], layer2[pe + 6], layer3[pe + 1], layer3[pe + 6]);
        swap_and_merge(layer2[pe + 2], layer2[pe + 5], layer3[pe + 2], layer3[pe + 5]);
        swap_and_merge(layer2[pe + 3], layer2[pe + 4], layer3[pe + 3], layer3[pe + 4]);
    }

    DenseVal_t layer4[NUM_PES];
    #pragma HLS array_partition variable=layer4 type=complete
    
    pipeline_layer4:
    for (int pe = 0; pe < NUM_PES; pe += 4) {
        #pragma HLS UNROLL
        swap_and_merge(layer3[pe], layer3[pe + 2], layer4[pe], layer4[pe + 2]);
        swap_and_merge(layer3[pe + 1], layer3[pe + 3], layer4[pe + 1], layer4[pe + 3]);
    }

    DenseVal_t layer5[NUM_PES];
    #pragma HLS array_partition variable=layer5 type=complete
    
    pipeline_layer5:
    for (int pe = 0; pe < NUM_PES; pe += 2) {
        #pragma HLS UNROLL
        swap_and_merge(layer4[pe], layer4[pe + 1], layer5[pe], layer5[pe + 1]);
    }

    DenseVal_t layer6[NUM_PES];
    #pragma HLS array_partition variable=layer6 type=complete

    swap_and_merge(layer5[0], layer5[15], layer6[0], layer6[15]);
    swap_and_merge(layer5[1], layer5[14], layer6[1], layer6[14]);
    swap_and_merge(layer5[2], layer5[13], layer6[2], layer6[13]);
    swap_and_merge(layer5[3], layer5[12], layer6[3], layer6[12]);
    swap_and_merge(layer5[4], layer5[11], layer6[4], layer6[11]);
    swap_and_merge(layer5[5], layer5[10], layer6[5], layer6[10]);
    swap_and_merge(layer5[6], layer5[9], layer6[6], layer6[9]);
    swap_and_merge(layer5[7], layer5[8], layer6[7], layer6[8]);

    DenseVal_t layer7[NUM_PES];
    #pragma HLS array_partition variable=layer7 type=complete
    
    pipeline_layer7:
    for (int pe = 0; pe < NUM_PES; pe += 8) {
        #pragma HLS UNROLL
        swap_and_merge(layer6[pe], layer6[pe + 4], layer7[pe], layer7[pe + 4]);
        swap_and_merge(layer6[pe + 1], layer6[pe + 5], layer7[pe + 1], layer7[pe + 5]);
        swap_and_merge(layer6[pe + 2], layer6[pe + 6], layer7[pe + 2], layer7[pe + 6]);
        swap_and_merge(layer6[pe + 3], layer6[pe + 7], layer7[pe + 3], layer7[pe + 7]);
    }

    DenseVal_t layer8[NUM_PES];
    #pragma HLS array_partition variable=layer8 type=complete
    
    pipeline_layer8:
    for (int pe = 0; pe < NUM_PES; pe += 4) {
        #pragma HLS UNROLL
        swap_and_merge(layer7[pe], layer7[pe + 2], layer8[pe], layer8[pe + 2]);
        swap_and_merge(layer7[pe + 1], layer7[pe + 3], layer8[pe + 1], layer8[pe + 3]);
    }

    pipeline_layer9:
    for (int pe = 0; pe < NUM_PES; pe += 2) {
        #pragma HLS UNROLL
        swap_and_merge(layer8[pe], layer8[pe + 1], out_array[pe], out_array[pe + 1]);
    }
}
#endif

inline void write_back(DenseVal_t (&out_array)[NUM_PES], ap_uint<32> *out_data,
                       int &atomic_write_counter) {
    write_back_loop:
    for (int wire = 0; wire < NUM_PES; wire++) {
        if (out_array[wire].valid) {
            ap_uint<32> index = out_array[wire].dest;
            out_data[index] = out_array[wire].val;
            atomic_write_counter++;
        }
    }
}
