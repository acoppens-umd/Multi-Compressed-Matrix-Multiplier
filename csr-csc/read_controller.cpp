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

    static ap_uint<NUM_PES> a_streaming = 0;
    static ap_uint<32> a_stream_counter[NUM_PES];
    #pragma HLS ARRAY_PARTITION variable=a_stream_counter dim=1 type=complete
    static ap_uint<32> a_stream_end[NUM_PES];
    #pragma HLS ARRAY_PARTITION variable=a_stream_end dim=1 type=complete

    serve_a_reads:
    while (a_pes_active != 0) {
        for (int pe = 0; pe < NUM_PES; pe++) {
            #pragma HLS pipeline style=flp
            if (a_pes_active(pe, pe) == 1) {
                if (a_streaming(pe, pe) == 0) {
                    AInPortReadReq_t a_read_req;

                    if (a_position_stream[pe].read_nb(a_read_req)) {
                        if (a_read_req.end) {
                            a_pes_active(pe, pe) = 0;
                        } else {
                            a_streaming(pe, pe) = 1;
                            a_stream_counter[pe] = a_read_req.start_indptr;
                            a_stream_end[pe] = a_read_req.end_indptr;
                        }
                    }                
                } else {
                    if (a_stream_counter[pe] >= a_stream_end[pe]) {
                        a_streaming(pe, pe) = 0;
                    } else if (a_data_stream[pe].size() < a_data_stream[pe].capacity()) {
                        a_data_stream[pe].write(a_in_port[a_stream_counter[pe]]);
                        a_stream_counter[pe]++;
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

    static ap_uint<NUM_PES> b_streaming = 0;
    static ap_uint<32> b_stream_counter[NUM_PES];
    #pragma HLS ARRAY_PARTITION variable=b_stream_counter dim=1 type=complete
    static ap_uint<32> b_stream_end[NUM_PES];
    #pragma HLS ARRAY_PARTITION variable=b_stream_end dim=1 type=complete

    serve_b_reads:
    while (b_pes_active != 0) {
        for (int pe = 0; pe < NUM_PES; pe++) {
            #pragma HLS pipeline style=flp
            if (b_pes_active(pe, pe) == 1) {
                if (b_streaming(pe, pe) == 0) {
                    BInPortReadReq_t b_read_req;

                    if (b_position_stream[pe].read_nb(b_read_req)) {
                        if (b_read_req.end) {
                            b_pes_active(pe, pe) = 0;
                        } else {
                            b_streaming(pe, pe) = 1;
                            b_stream_counter[pe] = b_read_req.start_indptr;
                            b_stream_end[pe] = b_read_req.end_indptr;
                        }
                    }                
                } else {
                    if (b_stream_counter[pe] >= b_stream_end[pe]) {
                        b_streaming(pe, pe) = 0;
                    } else if (b_data_stream[pe].size() < b_data_stream[pe].capacity()) {
                        b_data_stream[pe].write(b_in_port[b_stream_counter[pe]]);
                        b_stream_counter[pe]++;
                    }    
                }
            }
        }
    }
}