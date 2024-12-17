#include "task_distributor.h"

void task_distributor(
    const int a_num_rows,
    const int b_num_cols, 

    const a_meta_in_port_t *a_meta_in_port,
    const b_meta_in_port_t *b_meta_in_port,
    
    //Control flow signals to PEs
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]

) {

    ap_uint<32> a_row_indptr_memory[MAX_A_ROWS];
    ap_uint<32> b_col_indptr_memory[MAX_B_COLS];

    //Read locations for dense elements and send to task distributor
    read_sparse_meta_data(a_num_rows, b_num_cols, 
                                    a_meta_in_port, 
                                    a_row_indptr_memory,
                                    b_meta_in_port, 
                                    b_col_indptr_memory);

    send_elements(a_num_rows, b_num_cols,
                  a_row_indptr_memory,
                  b_col_indptr_memory,
                  task_a_position_streams,
                  task_b_position_streams);

}

inline void read_sparse_meta_data(const int a_num_rows, const int b_num_cols, 
                                    const a_meta_in_port_t *a_meta_in_port, 
                                    ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS],
                                    const b_meta_in_port_t *b_meta_in_port, 
                                    ap_uint<32> (&b_col_indptr_memory)[MAX_B_COLS]) {
#pragma HLS DATAFLOW
    read_a_row_indptrs(a_meta_in_port, a_num_rows,
                      a_row_indptr_memory);

    read_b_col_indptrs(b_meta_in_port, b_num_cols,
                      b_col_indptr_memory);
}

inline void read_a_row_indptrs(const a_meta_in_port_t *a_meta_in_port, int a_num_rows,
                                ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS]) {
    read_a_rows: for (int a_count = 0; a_count < a_num_rows + 1; a_count++) {
        a_row_indptr_memory[a_count] = a_meta_in_port[a_count];
    }
}

inline void read_b_col_indptrs(const b_meta_in_port_t *b_meta_in_port, int b_num_cols, 
                                ap_uint<32> (&b_col_indptr_memory)[MAX_B_COLS]) {
    read_b_cols: for (int b_count = 0; b_count < b_num_cols + 1; b_count++) {
        b_col_indptr_memory[b_count] = b_meta_in_port[b_count];
    }
}


inline void send_elements(int a_num_rows, int b_num_cols,
                          ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS],
                          ap_uint<32> (&b_col_indptr_memory)[MAX_B_COLS],
                          hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
                          hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]) {
    int a_row_ctr = 0;
    int b_col_ctr = 0;
    
    //Send pairs of elements to PEs as tasks
    send_pairs:
    while (a_row_ctr < a_num_rows) {        
        for (int pe = 0; pe < NUM_PES; pe++) {
            if (a_row_ctr < a_num_rows && 
                task_a_position_streams[pe].size() < task_a_position_streams[pe].capacity()) {
                
                task_a_position_streams[pe].write({
                    a_row_ctr,
                    a_row_indptr_memory[a_row_ctr], 
                    a_row_indptr_memory[a_row_ctr + 1],
                    false
                });
                task_b_position_streams[pe].write({
                    b_col_ctr,
                    b_col_indptr_memory[b_col_ctr], 
                    b_col_indptr_memory[b_col_ctr + 1],
                    false
                });

                b_col_ctr++;
                if (b_col_ctr == b_num_cols) {
                    a_row_ctr++;
                    b_col_ctr = 0;
                }
            }
        }
    }

    //Send last signal to finalize PEs
    for (int pe = 0; pe < NUM_PES; pe++) {
        task_a_position_streams[pe].write({0, 0, 0, true});
        task_b_position_streams[pe].write({0, 0, 0, true});
    }
}