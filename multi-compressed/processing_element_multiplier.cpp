#include "processing_element_multiplier.h"
#include "cache.h"

void processing_element_multiplier(
    //in
    hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> &a_data_stream, //from read_controller 
    hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> &b_data_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream, //from PE
    
    //out
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> &out_stream
) {
    a_in_port_t a_cache[A_CACHE_SIZE];
    b_in_port_t b_cache[B_CACHE_SIZE];

    bool active = true;
    bool first = true;
    ap_uint<32> dest_i = 0;
    ap_uint<32> dest_k = 0;
    float accumulator = 0.0f;

    multiplier_loop:
    while (active) {
        AtomicInstruction_t instruction = instruction_stream.read();

        if (instruction.end == true) {
            if (first == false) {
                bit_float out_convert;
                out_convert.as_float32 = accumulator;

                DenseVal_t output_dense_val;
                output_dense_val.val = out_convert.as_uint32;
                output_dense_val.dest_i = dest_i;
                output_dense_val.dest_k = dest_k;
                output_dense_val.valid = true;

                out_stream.write(output_dense_val);

                accumulator = 0.0f;
            }

            active = false;
        } else {
            //If destination has changed then flush to write network
            send_partial_product: if (first == false && (instruction.dest_i != dest_i || instruction.dest_k != dest_k)) {
                bit_float out_convert;
                out_convert.as_float32 = accumulator;

                DenseVal_t output_dense_val;
                output_dense_val.val = out_convert.as_uint32;
                output_dense_val.dest_i = dest_i;
                output_dense_val.dest_k = dest_k;
                output_dense_val.valid = true;

                out_stream.write(output_dense_val);

                accumulator = 0.0f;
            }

            first = false;
            dest_i = instruction.dest_i;
            dest_k = instruction.dest_k;
            
            ap_uint<A_CACHE_ADDR_WIDTH> a_cache_index = instruction.a_addr(A_CACHE_ADDR_WIDTH - 1, 0);

            if (!instruction.use_cached_a) {
                a_in_port_t a_data = a_data_stream.read();
                a_cache[a_cache_index] = a_data;
            }

            ap_uint<B_CACHE_ADDR_WIDTH> b_cache_index = instruction.b_addr(B_CACHE_ADDR_WIDTH - 1, 0);

            if (!instruction.use_cached_b) {
                b_in_port_t b_data = b_data_stream.read();
                b_cache[b_cache_index] = b_data;
            }
            
            bit_float a_float; 
            a_float.as_uint32 = a_cache[a_cache_index](32 * (instruction.a_offset + 1) - 1, 32 * instruction.a_offset);
            bit_float b_float;
            b_float.as_uint32 = b_cache[b_cache_index](32 * (instruction.b_offset + 1) - 1, 32 * instruction.b_offset);

            accumulator += a_float.as_float32 * b_float.as_float32;
        }
    }

    DenseVal_t final_dense_val;
    final_dense_val.val = 0.0f;
    final_dense_val.dest_i = 0;
    final_dense_val.dest_k = 0;
    final_dense_val.valid = false;

    out_stream.write(final_dense_val);
}