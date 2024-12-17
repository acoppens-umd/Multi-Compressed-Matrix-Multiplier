#include "write_back_gate.h"

void write_back_gate(
    //in
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&in_streams)[NUM_PES],

    //out
    hls::stream<bool, DEFAULT_STREAM_DEPTH> &gate_to_network_initiation_stream,
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> (&gate_to_network_streams)[NUM_PES]
) {
    static ap_uint<NUM_PES> active_pes = -1;

    gate_loop:
    while (active_pes != 0) {
        DenseVal_t dense_package[NUM_PES];
        #pragma HLS array_partition variable=dense_package type=complete

        ap_uint<NUM_PES> valid = 0;

        read_into_gate_loop:
        for (int pe = 0; pe < NUM_PES; pe++) {
            #pragma HLS UNROLL

            if (in_streams[pe].read_nb(dense_package[pe])) {
                if (!dense_package[pe].valid) {
                    active_pes(pe, pe) = 0;
                } else {
                    valid(pe, pe) = 1;
                }
            } else {
                dense_package[pe].valid = false;
            }
        }

        if (valid != 0) {
            gate_to_network_initiation_stream.write(true);

            for (int pe = 0; pe < NUM_PES; pe++) {
                #pragma HLS UNROLL

                gate_to_network_streams[pe].write(dense_package[pe]);
            }
        }
    }

    //Write final message to tell write_back_network to end
    gate_to_network_initiation_stream.write(false);

    for (int pe = 0; pe < NUM_PES; pe++) {
        #pragma HLS UNROLL

        gate_to_network_streams[pe].write({0.0f, 0, false});
    }
}