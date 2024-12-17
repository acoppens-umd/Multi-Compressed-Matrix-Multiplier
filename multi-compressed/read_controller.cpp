#include "read_controller.h"

void read_controller(
    //in
    const a_in_port_t *a_in_port,
    const b_in_port_t *b_in_port,

    //Streaming requests from PE1s
    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&a_position_stream)[NUM_PES],
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&b_position_stream)[NUM_PES],

    //Data towards PE2s
    hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> (&a_data_stream)[NUM_PES],
    hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> (&b_data_stream)[NUM_PES]
) {
    
    //Receive read addresses from processing elements and send to processing elements
    #pragma HLS DATAFLOW
    serve_a_reads(a_in_port, a_position_stream, a_data_stream);

    serve_b_reads(b_in_port, b_position_stream, b_data_stream);
}

inline void serve_a_reads(const a_in_port_t *a_in_port,
                          hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&a_position_stream)[NUM_PES],
                          hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> (&a_data_stream)[NUM_PES]) {
    static ap_uint<NUM_PES> a_pes_active = -1;

    serve_a_reads:
    while (a_pes_active != 0) {
        for (int pe = 0; pe < NUM_PES; pe++) {
            if (a_pes_active(pe, pe) == 1) {
                AInPortReadReq_t a_read_req;

                if (a_position_stream[pe].read_nb(a_read_req)) {
                    if (a_read_req.end) {
                        a_pes_active(pe, pe) = 0;
                    } else {
                        a_data_stream[pe].write(a_in_port[a_read_req.addr]);
                    }
                }                
            }
        }
    }
}

inline void serve_b_reads(const b_in_port_t *b_in_port,
                          hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> (&b_position_stream)[NUM_PES],
                          hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> (&b_data_stream)[NUM_PES]) {
    static ap_uint<NUM_PES> b_pes_active = -1;

    serve_b_reads:
    while (b_pes_active != 0) {
        for (int pe = 0; pe < NUM_PES; pe++) {
            if (b_pes_active(pe, pe) == 1) {
                BInPortReadReq_t b_read_req;

                if (b_position_stream[pe].read_nb(b_read_req)) {
                    if (b_read_req.end) {
                        b_pes_active(pe, pe) = 0;
                    } else {
                        b_data_stream[pe].write(b_in_port[b_read_req.addr]);
                    }
                }                
            }
        }
    }
}