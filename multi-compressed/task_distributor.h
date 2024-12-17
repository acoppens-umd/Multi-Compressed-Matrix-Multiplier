#ifndef task_distributor_h
#define task_distributor_h

#include "kernel.h"
#include "types.h"

inline void read_dense_diag_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, 
                                      TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                      int a_dense_diag_start_addr,
                                      const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, 
                                      TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                      int b_dense_diag_start_addr);

inline void read_dense_a_diag_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, 
                                        TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                        int a_dense_diag_start_addr);

inline void read_dense_b_diag_addresses(const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, 
                                        TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                        int b_dense_diag_start_addr);

inline void read_dense_block_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, int a_num_dense_blocks, 
                                      TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                      int a_dense_block_start_addr,
                                      const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, int b_num_dense_blocks, 
                                      TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                      int b_dense_block_start_addr);

inline void read_dense_a_block_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, int a_num_dense_blocks, 
                                         TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                         int a_dense_block_start_addr);

inline void read_dense_b_block_addresses(const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, int b_num_dense_blocks, 
                                         TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                         int b_dense_block_start_addr);

inline void send_elements(int a_dense_elements, int b_dense_elements,
                          TaskAPosition_t *a_dense_element_memory,
                          TaskBPosition_t *b_dense_element_memory,
                          hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
                          hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]);

void task_distributor(
    const int a_num_dense_diags,
    const int a_num_dense_blocks,
    const int b_num_dense_diags,
    const int b_num_dense_blocks,

    const int a_dense_diag_start_addr,
    const int b_dense_diag_start_addr,
    const int a_dense_block_start_addr,
    const int b_dense_block_start_addr, 
    
    const a_meta_in_port_t *a_meta_in_port,
    const b_meta_in_port_t *b_meta_in_port,
    
    //Control flow signals to PEs
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]
);

#endif