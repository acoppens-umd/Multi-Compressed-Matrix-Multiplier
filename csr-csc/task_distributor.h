#ifndef task_distributor_h
#define task_distributor_h

#include "kernel.h"
#include "types.h"

inline void read_sparse_meta_data(const int a_num_rows, const int b_num_cols, 
                                    const a_meta_in_port_t *a_meta_in_port, 
                                    ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS],
                                    const b_meta_in_port_t *b_meta_in_port, 
                                    ap_uint<32> (&b_col_indptr_memory)[MAX_B_COLS]);

inline void read_a_row_indptrs(const a_meta_in_port_t *a_meta_in_port, int a_num_rows,
                                ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS]);

inline void read_b_col_indptrs(const b_meta_in_port_t *b_meta_in_port, int b_num_cols, 
                                ap_uint<32>  (&b_col_indptr_memory)[MAX_B_COLS]);

inline void send_elements(int a_num_rows, int b_num_cols,
                          ap_uint<32> (&a_row_indptr_memory)[MAX_A_ROWS],
                          ap_uint<32> (&b_col_indptr_memory)[MAX_B_COLS],
                          hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
                          hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]);

void task_distributor(
    const int a_num_rows,
    const int b_num_cols,  
    
    const a_meta_in_port_t *a_meta_in_port,
    const b_meta_in_port_t *b_meta_in_port,
    
    //Control flow signals to PEs
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]
);

#endif