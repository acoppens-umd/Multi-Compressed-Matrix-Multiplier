#include "processing_element_multiplier.h"

void processing_element_multiplier(
    const int b_num_cols,
    //in
    hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> &a_data_stream, //from read_controller 
    hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> &b_data_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream, //from PE
    
    //out
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> &out_stream
) {

    bool active = true;

    multiplier_loop:
    while (active) {
        AtomicInstruction_t instruction = instruction_stream.read();

        if (instruction.end == true) {
            active = false;
        } else {
            float accumulator = 0.0f;
            ap_uint<32> a_counter = instruction.a_start_indptr;
            ap_uint<32> b_counter = instruction.b_start_indptr;

            ap_uint<32> out_index = instruction.row * b_num_cols + instruction.col;

            a_in_port_t a_data = a_data_stream.read();
            b_in_port_t b_data = b_data_stream.read();

            dot_product_loop: 
            while (a_counter < instruction.a_end_indptr && b_counter < instruction.b_end_indptr) {
                if (a_data(63, 32) == b_data(63, 32)) {
                    bit_float a_bf;
                    a_bf.as_uint32 = a_data(31, 0);

                    bit_float b_bf;
                    b_bf.as_uint32 = b_data(31, 0);

                    accumulator += a_bf.as_float32 * b_bf.as_float32;

                    a_counter++;
                    if (a_counter < instruction.a_end_indptr) {
                        a_data = a_data_stream.read();
                    }

                    b_counter++;
                    if (b_counter < instruction.b_end_indptr) {
                        b_data = b_data_stream.read();
                    }
                } else if (a_data(63, 32) < b_data(63, 32)) {
                    a_counter++;
                    if (a_counter < instruction.a_end_indptr) {
                        a_data = a_data_stream.read();
                    }
                } else {
                    b_counter++;
                    if (b_counter < instruction.b_end_indptr) {
                        b_data = b_data_stream.read();
                    }
                }
            }

            bit_float out_convert;
            out_convert.as_float32 = accumulator;

            DenseVal_t output_dense_val;
            output_dense_val.val = out_convert.as_uint32;
            output_dense_val.dest = out_index;
            output_dense_val.valid = true;

            out_stream.write(output_dense_val);

            flush_a_stream:
            while (a_counter < instruction.a_end_indptr) {
                a_counter++;
                if (a_counter < instruction.a_end_indptr) {
                    a_data = a_data_stream.read();
                }
            }

            flush_b_stream:
            while (b_counter < instruction.b_end_indptr) {
                b_counter++;
                if (b_counter < instruction.b_end_indptr) {
                    b_data = b_data_stream.read();
                }
            }
        }
    }

    DenseVal_t final_dense_val;
    final_dense_val.val = 0.0f;
    final_dense_val.dest = 0;
    final_dense_val.valid = false;

    out_stream.write(final_dense_val);
}