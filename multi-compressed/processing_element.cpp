#include "processing_element.h"
#include "util.h"

void processing_element(
    const int pe,

    //in
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> &task_a_position_stream, //from job distributor
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> &task_b_position_stream,

    ap_uint<32> a_rows,
    ap_uint<32> a_cols,
    ap_uint<32> b_rows,
    ap_uint<32> b_cols,
    
    //out
    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream
) {
    static ACacheLine a_cache_memory[NUM_PES][A_CACHE_SIZE] = {false, 0};
    static BCacheLine b_cache_memory[NUM_PES][B_CACHE_SIZE] = {false, 0};
    #pragma HLS array_partition variable=a_cache_memory type=complete dim=1
    #pragma HLS array_partition variable=b_cache_memory type=complete dim=1

    bool last_task = false;

    compute_loop: do {
        TaskAPosition_t task_a_position = task_a_position_stream.read();
        TaskBPosition_t task_b_position = task_b_position_stream.read();

        a_in_port_addr_t task_a_addr = task_a_position.addr;
        ap_uint<32> task_a_start_i = task_a_position.start_i;
        ap_uint<32> task_a_start_j = task_a_position.start_j;
        ap_uint<32> task_a_end_i = task_a_position.end_i;
        ap_uint<32> task_a_end_j = task_a_position.end_j;

        b_in_port_addr_t task_b_addr = task_b_position.addr;
        ap_uint<32> task_b_start_j = task_b_position.start_j;
        ap_uint<32> task_b_start_k = task_b_position.start_k;
        ap_uint<32> task_b_end_j = task_b_position.end_j;
        ap_uint<32> task_b_end_k = task_b_position.end_k;

        pe_computation_mode computation_mode;

        if (task_a_position.end) {
            last_task = true;
            computation_mode = FINISH;
        } else if (task_a_end_i == 0 && task_b_end_j == 0) {
            computation_mode = DENSE_DIAG_DENSE_DIAG;
        } else if (task_a_end_i == 0) {
            computation_mode = DENSE_DIAG_DENSE_BLOCK;
        } else if (task_b_end_j == 0) {
            computation_mode = DENSE_BLOCK_DENSE_DIAG;
        } else {
            computation_mode = DENSE_BLOCK_DENSE_BLOCK;            
        }
        
        if (computation_mode == DENSE_DIAG_DENSE_DIAG) {
            ap_uint<32> out_row = task_a_start_i;
            ap_uint<32> out_col = task_b_start_k;

            ap_uint<32> a_start_offset = 0;
            ap_uint<32> b_start_offset = 0;

            ap_uint<32> a_end_offset = 0;
            ap_uint<32> b_end_offset = 0;

            if (task_a_start_j > task_b_start_j) {
                ap_uint<32> diff = task_a_start_j - task_b_start_j;
                out_col += diff;
                b_start_offset = diff;
            } else if (task_a_start_j < task_b_start_j) {
                ap_uint<32> diff = task_b_start_j - task_a_start_j;
                out_row += diff;
                a_start_offset = diff;
            }            

            ap_uint<32> a_diag_row_len = a_rows - task_a_start_i;
            ap_uint<32> a_diag_col_len = a_cols - task_a_start_j;
            ap_uint<32> b_diag_row_len = b_rows - task_b_start_j;
            ap_uint<32> b_diag_col_len = b_cols - task_b_start_k;

            min(a_diag_row_len, a_diag_col_len, a_end_offset);
            min(b_diag_row_len, b_diag_col_len, b_end_offset);

            ap_uint<32> a_offset_ctr = a_start_offset;
            ap_uint<32> b_offset_ctr = b_start_offset;

            //Pair up values from diagonals to produce single product partial sums
            diag_diag_mult_loop: 
            while (a_offset_ctr < a_end_offset && b_offset_ctr < b_end_offset) {
                a_in_port_addr_t read_a_addr = task_a_addr + (a_offset_ctr >> 2);
                b_in_port_addr_t read_b_addr = task_b_addr + (b_offset_ctr >> 2);
                ap_uint<2> a_offset_remainder = a_offset_ctr(1, 0);
                ap_uint<2> b_offset_remainder = b_offset_ctr(1, 0);

                cache_aware_send(a_cache_memory[pe], b_cache_memory[pe],
                                    out_row, out_col,
                                    read_a_addr, read_b_addr,
                                    a_offset_remainder, b_offset_remainder,
                                    a_read_addr_stream, b_read_addr_stream,
                                    instruction_stream);

                a_offset_ctr++;
                b_offset_ctr++;

                out_row++;
                out_col++;
            }
        } else if (computation_mode == DENSE_DIAG_DENSE_BLOCK) {
            ap_uint<32> task_a_end_j_short = task_a_start_j + a_rows - task_a_start_i;
            min(a_cols, task_a_end_j_short, task_a_end_j);

            ap_uint<32> interval_j_start;
            ap_uint<32> interval_j_end;

            max(task_a_start_j, task_b_start_j, interval_j_start);
            min(task_a_end_j, task_b_end_j, interval_j_end);

            if (interval_j_start < interval_j_end) {
                ap_uint<32> a_offset_start = interval_j_start - task_a_start_j;
                ap_uint<32> a_offset_ctr = a_offset_start;

                ap_uint<32> b_k_width = task_b_end_k - task_b_start_k;
                ap_uint<32> b_offset_start = b_k_width * (interval_j_start - task_b_start_j);
                ap_uint<32> b_offset_ctr = b_offset_start;

                //Loop over scalars of diag and rows of b block
                for (ap_uint<32> j = interval_j_start; j < interval_j_end; j++) {

                    //Loop produces a row via multiplication of a row of b block with one scalar from diag
                    for (ap_uint<32> k = task_b_start_k; k < task_b_end_k; k++) {
                        a_in_port_addr_t read_a_addr = task_a_addr + (a_offset_ctr >> 2);
                        b_in_port_addr_t read_b_addr = task_b_addr + (b_offset_ctr >> 2);
                        ap_uint<2> a_offset_remainder = a_offset_ctr(1, 0);
                        ap_uint<2> b_offset_remainder = b_offset_ctr(1, 0);
                        ap_uint<32> i = task_a_start_i + a_offset_ctr;

                        cache_aware_send(a_cache_memory[pe], b_cache_memory[pe],
                                            i, k,
                                            read_a_addr, read_b_addr,
                                            a_offset_remainder, b_offset_remainder,
                                            a_read_addr_stream, b_read_addr_stream,
                                            instruction_stream);

                        b_offset_ctr++;
                    }

                    a_offset_ctr++;
                }
            }

        } else if (computation_mode == DENSE_BLOCK_DENSE_DIAG) {
            ap_uint<32> task_b_end_j_short = task_b_start_j + b_cols - task_b_start_k;
            min(b_rows, task_b_end_j_short, task_b_end_j);

            ap_uint<32> interval_j_start;
            ap_uint<32> interval_j_end;

            max(task_a_start_j, task_b_start_j, interval_j_start);
            min(task_a_end_j, task_b_end_j, interval_j_end);

            if (interval_j_start < interval_j_end) {
                ap_uint<32> a_offset_start = interval_j_start - task_a_start_j;
                ap_uint<32> b_offset_start = interval_j_start - task_b_start_j;
                
                ap_uint<32> a_j_width = task_a_end_j - task_a_start_j;

                //loop over rows of a block
                for (ap_uint<32> i = task_a_start_i; i < task_a_end_i; i++) {
                    ap_uint<32> a_offset_ctr = a_offset_start;
                    ap_uint<32> b_offset_ctr = b_offset_start;

                    //loop over scalars of b diag
                    for (ap_uint<32> j = interval_j_start; j < interval_j_end; j++) {
                        a_in_port_addr_t read_a_addr = task_a_addr + (a_offset_ctr >> 2);
                        b_in_port_addr_t read_b_addr = task_b_addr + (b_offset_ctr >> 2);
                        ap_uint<2> a_offset_remainder = a_offset_ctr(1, 0);
                        ap_uint<2> b_offset_remainder = b_offset_ctr(1, 0);
                        ap_uint<32> k = task_b_start_k + b_offset_ctr;

                        cache_aware_send(a_cache_memory[pe], b_cache_memory[pe],
                                            i, k,
                                            read_a_addr, read_b_addr,
                                            a_offset_remainder, b_offset_remainder,
                                            a_read_addr_stream, b_read_addr_stream,
                                            instruction_stream);
                    
                        a_offset_ctr++;
                        b_offset_ctr++;
                    }

                    a_offset_start += a_j_width;
                }
            }

        } else if (computation_mode == DENSE_BLOCK_DENSE_BLOCK) {
            ap_uint<32> dot_j_start;
            ap_uint<32> dot_j_end;

            max(task_a_start_j, task_b_start_j, dot_j_start);
            min(task_a_end_j, task_b_end_j, dot_j_end);
            
            //Skip if j indices don't align
            if (dot_j_start < dot_j_end) {
                
                ap_uint<32> a_j_width = task_a_end_j - task_a_start_j;
                ap_uint<32> b_k_width = task_b_end_k - task_b_start_k;

                ap_uint<32> a_offset_start = dot_j_start - task_a_start_j;
                ap_uint<32> b_offset_start = b_k_width * (dot_j_start - task_b_start_j);
                
                for (ap_uint<32> i = task_a_start_i; i < task_a_end_i; i++) {
                    for (ap_uint<32> k = task_b_start_k; k < task_b_end_k; k++) {
                        ap_uint<32> a_offset_ctr = a_offset_start;
                        ap_uint<32> b_offset_ctr = b_offset_start;

                        for (int j = dot_j_start; j < dot_j_end; j++) {
                            a_in_port_addr_t read_a_addr = task_a_addr + (a_offset_ctr >> 2);
                            b_in_port_addr_t read_b_addr = task_b_addr + (b_offset_ctr >> 2);
                            ap_uint<2> a_offset_remainder = a_offset_ctr(1, 0);
                            ap_uint<2> b_offset_remainder = b_offset_ctr(1, 0);

                            cache_aware_send(a_cache_memory[pe], b_cache_memory[pe],
                                             i, k,
                                             read_a_addr, read_b_addr,
                                             a_offset_remainder, b_offset_remainder,
                                             a_read_addr_stream, b_read_addr_stream,
                                             instruction_stream);
                            
                            a_offset_ctr++;
                            b_offset_ctr += b_k_width;
                        }
                        
                        b_offset_start++;
                    }

                    b_offset_start -= b_k_width;
                    a_offset_start += a_j_width;
                }
            }
        }
        

    } while (!last_task);

    a_read_addr_stream.write({0, true});
    b_read_addr_stream.write({0, true});

    instruction_stream.write({0, 0, 0, 0, 0, 0, false, false, true});
}

inline void cache_aware_send(ACacheLine (&a_cache_memory)[A_CACHE_SIZE], 
                             BCacheLine (&b_cache_memory)[B_CACHE_SIZE],
                             ap_uint<32> &dest_i, ap_uint<32> &dest_k,
                             a_in_port_addr_t &a_addr, b_in_port_addr_t &b_addr,
                             ap_uint<2> &a_offset, ap_uint<2> &b_offset,
                             hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
                             hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,
                             hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream) {
    AtomicInstruction_t instruction;
    instruction.dest_i = dest_i;
    instruction.dest_k = dest_k;
    instruction.a_addr = a_addr;
    instruction.b_addr = b_addr;
    instruction.a_offset = a_offset;
    instruction.b_offset = b_offset;
    instruction.end = false;

    ap_uint<A_CACHE_ADDR_WIDTH> a_index = a_addr(A_CACHE_ADDR_WIDTH - 1, 0); 
    ap_uint<A_TAG_WIDTH> a_tag = a_addr(A_MEMORY_SPACE_ADDR_WIDTH - 1, A_CACHE_ADDR_WIDTH);

    if (a_cache_memory[a_index].valid == false || a_cache_memory[a_index].tag != a_tag) {
        a_read_addr_stream.write({a_addr, false});
        
        a_cache_memory[a_index].valid = true;
        a_cache_memory[a_index].tag = a_tag;

        instruction.use_cached_a = false;
    } else {
        instruction.use_cached_a = true;
    }

    ap_uint<B_CACHE_ADDR_WIDTH> b_index = b_addr(B_CACHE_ADDR_WIDTH - 1, 0); 
    ap_uint<B_TAG_WIDTH> b_tag = b_addr(B_MEMORY_SPACE_ADDR_WIDTH - 1, B_CACHE_ADDR_WIDTH);

    if (b_cache_memory[b_index].valid == false || b_cache_memory[b_index].tag != b_tag) {
        b_read_addr_stream.write({b_addr, false});
        
        b_cache_memory[b_index].valid = true;
        b_cache_memory[b_index].tag = b_tag;

        instruction.use_cached_b = false;
    } else {
        instruction.use_cached_b = true;
    }

    instruction_stream.write(instruction);
}