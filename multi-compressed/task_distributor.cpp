#include "task_distributor.h"

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

) {
    int a_dense_elements = a_num_dense_diags + a_num_dense_blocks;
    int b_dense_elements = b_num_dense_diags + b_num_dense_blocks;

    TaskAPosition_t a_dense_element_memory[MAX_A_DENSE_OBJECTS];
    TaskBPosition_t b_dense_element_memory[MAX_B_DENSE_OBJECTS];

    //Read locations for dense elements and send to task distributor
    read_dense_diag_addresses(a_meta_in_port, a_num_dense_diags, a_dense_element_memory,
                              a_dense_diag_start_addr,
                              b_meta_in_port, b_num_dense_diags, b_dense_element_memory,
                              b_dense_diag_start_addr);

    read_dense_block_addresses(a_meta_in_port, a_num_dense_diags, a_num_dense_blocks, a_dense_element_memory,
                               a_dense_block_start_addr,
                               b_meta_in_port, b_num_dense_diags, b_num_dense_blocks, b_dense_element_memory,
                               b_dense_block_start_addr);

    send_elements(a_dense_elements, b_dense_elements,
                  a_dense_element_memory,
                  b_dense_element_memory,
                  task_a_position_streams,
                  task_b_position_streams);

}

inline void read_dense_diag_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, 
                                      TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                      int a_dense_diag_start_addr,
                                      const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, 
                                      TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                      int b_dense_diag_start_addr) {
#pragma HLS DATAFLOW
    read_dense_a_diag_addresses(a_meta_in_port, a_num_dense_diags, 
                                a_dense_element_memory, a_dense_diag_start_addr);

    read_dense_b_diag_addresses(b_meta_in_port, b_num_dense_diags, 
                                b_dense_element_memory, b_dense_diag_start_addr);
}

inline void read_dense_a_diag_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, 
                                        TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                        int a_dense_diag_start_addr) {
    int a_diag_cyclic_ctr = 0;
    int a_diag_ctr = 0;

    a_in_port_addr_t diag_a_addr;
    ap_uint<32> diag_a_start_i;
    ap_uint<32> diag_a_start_j;

    read_a_diagonals: for (int a_count = 0; a_count < a_num_dense_diags * 3; a_count++) {
        if (a_diag_cyclic_ctr == 0) {
            diag_a_addr = a_meta_in_port[a_dense_diag_start_addr + a_count];
        } else if (a_diag_cyclic_ctr == 1) {
            diag_a_start_i = a_meta_in_port[a_dense_diag_start_addr + a_count];
        } else if (a_diag_cyclic_ctr == 2) {
            diag_a_start_j = a_meta_in_port[a_dense_diag_start_addr + a_count];

            TaskAPosition_t task_a_position;

            task_a_position.addr    = diag_a_addr;
            task_a_position.start_i = diag_a_start_i;
            task_a_position.start_j = diag_a_start_j;
            task_a_position.end_i   = 0;
            task_a_position.end_j   = 0;
            task_a_position.end     = false;

            a_dense_element_memory[a_diag_ctr] = task_a_position;       
            a_diag_ctr++;
        } 

        a_diag_cyclic_ctr++;
        if (a_diag_cyclic_ctr == 3) {
            a_diag_cyclic_ctr = 0;
        }
    }
}

inline void read_dense_b_diag_addresses(const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, 
                                        TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                        int b_dense_diag_start_addr) {
    int b_diag_cyclic_ctr = 0;
    int b_diag_ctr = 0;

    b_in_port_addr_t diag_b_addr;
    ap_uint<32> diag_b_start_j;
    ap_uint<32> diag_b_start_k;

    read_b_diagonals: for (int b_count = 0; b_count < b_num_dense_diags * 3; b_count++) {
        if (b_diag_cyclic_ctr == 0) {
            diag_b_addr = b_meta_in_port[b_dense_diag_start_addr + b_count];
        } else if (b_diag_cyclic_ctr == 1) {
            diag_b_start_j = b_meta_in_port[b_dense_diag_start_addr + b_count];
        } else if (b_diag_cyclic_ctr == 2) {
            diag_b_start_k = b_meta_in_port[b_dense_diag_start_addr + b_count];

            TaskBPosition_t task_b_position;

            task_b_position.addr    = diag_b_addr;
            task_b_position.start_j = diag_b_start_j;
            task_b_position.start_k = diag_b_start_k;
            task_b_position.end_j   = 0;
            task_b_position.end_k   = 0;
            task_b_position.end     = false;

            b_dense_element_memory[b_diag_ctr] = task_b_position;    

            b_diag_ctr++;           
        } 

        b_diag_cyclic_ctr++;
        if (b_diag_cyclic_ctr == 3) {
            b_diag_cyclic_ctr = 0;
        }
    }
}

inline void read_dense_block_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, int a_num_dense_blocks, 
                                      TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                      int a_dense_block_start_addr,
                                      const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, int b_num_dense_blocks, 
                                      TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                      int b_dense_block_start_addr) {
#pragma HLS DATAFLOW
    read_dense_a_block_addresses(a_meta_in_port, a_num_dense_diags, a_num_dense_blocks, 
                                 a_dense_element_memory,
                                 a_dense_block_start_addr);

    read_dense_b_block_addresses(b_meta_in_port, b_num_dense_diags, b_num_dense_blocks, 
                                 b_dense_element_memory,
                                 b_dense_block_start_addr);
}

inline void read_dense_a_block_addresses(const a_meta_in_port_t *a_meta_in_port, int a_num_dense_diags, int a_num_dense_blocks, 
                                         TaskAPosition_t (&a_dense_element_memory)[MAX_A_DENSE_OBJECTS],
                                         int a_dense_block_start_addr) {
    
    int a_block_cyclic_ctr = 0;
    int a_block_ctr = a_num_dense_diags;

    a_in_port_addr_t block_a_addr;
    ap_uint<32> block_a_start_i;
    ap_uint<32> block_a_start_j;
    ap_uint<32> block_a_end_i;
    ap_uint<32> block_a_end_j;

    read_a_blocks: for (ap_uint<32> a_count = 0; a_count < a_num_dense_blocks * 5; a_count++) {
        if (a_block_cyclic_ctr == 0) {
            block_a_addr = a_meta_in_port[a_dense_block_start_addr + a_count];
        } else if (a_block_cyclic_ctr == 1) {
            block_a_start_i = a_meta_in_port[a_dense_block_start_addr + a_count];
        } else if (a_block_cyclic_ctr == 2) {
            block_a_start_j = a_meta_in_port[a_dense_block_start_addr + a_count];
        } else if (a_block_cyclic_ctr == 3) {
            block_a_end_i = a_meta_in_port[a_dense_block_start_addr + a_count];
        } else if (a_block_cyclic_ctr == 4) {
            block_a_end_j = a_meta_in_port[a_dense_block_start_addr + a_count];
            
            TaskAPosition_t task_a_position;

            task_a_position.addr    = block_a_addr;
            task_a_position.start_i = block_a_start_i;
            task_a_position.start_j = block_a_start_j;
            task_a_position.end_i   = block_a_end_i;
            task_a_position.end_j   = block_a_end_j;
            task_a_position.end     = false;

            a_dense_element_memory[a_block_ctr] = task_a_position;  
            a_block_ctr++;
        } 

        a_block_cyclic_ctr++;
        if (a_block_cyclic_ctr == 5) {
            a_block_cyclic_ctr = 0;
        }

    }
}

inline void read_dense_b_block_addresses(const b_meta_in_port_t *b_meta_in_port, int b_num_dense_diags, int b_num_dense_blocks, 
                                         TaskBPosition_t (&b_dense_element_memory)[MAX_B_DENSE_OBJECTS],
                                         int b_dense_block_start_addr) {
    int b_block_cyclic_ctr = 0;
    int b_block_ctr = b_num_dense_diags;

    b_in_port_addr_t block_b_addr;
    ap_uint<32> block_b_start_j;
    ap_uint<32> block_b_start_k;
    ap_uint<32> block_b_end_j;
    ap_uint<32> block_b_end_k;

    read_b_blocks: for (ap_uint<32> b_count = 0; b_count < b_num_dense_blocks * 5; b_count++) {
        if (b_block_cyclic_ctr == 0) {
            block_b_addr = b_meta_in_port[b_dense_block_start_addr + b_count];
        } else if (b_block_cyclic_ctr == 1) {
            block_b_start_j = b_meta_in_port[b_dense_block_start_addr + b_count];
        } else if (b_block_cyclic_ctr == 2) {
            block_b_start_k = b_meta_in_port[b_dense_block_start_addr + b_count];
        } else if (b_block_cyclic_ctr == 3) {
            block_b_end_j = b_meta_in_port[b_dense_block_start_addr + b_count];
        } else if (b_block_cyclic_ctr == 4) {
            block_b_end_k = b_meta_in_port[b_dense_block_start_addr + b_count];
            
            TaskBPosition_t task_b_position;

            task_b_position.addr    = block_b_addr;
            task_b_position.start_j = block_b_start_j;
            task_b_position.start_k = block_b_start_k;
            task_b_position.end_j   = block_b_end_j;
            task_b_position.end_k   = block_b_end_k;
            task_b_position.end     = false;

            b_dense_element_memory[b_block_ctr] = task_b_position;         

            b_block_ctr++; 
        } 

        b_block_cyclic_ctr++;
        if (b_block_cyclic_ctr == 5) {
            b_block_cyclic_ctr = 0;
        }
    }
}

inline void send_elements(int a_dense_elements, int b_dense_elements,
                          TaskAPosition_t *a_dense_element_memory,
                          TaskBPosition_t *b_dense_element_memory,
                          hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> (&task_a_position_streams)[NUM_PES],
                          hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> (&task_b_position_streams)[NUM_PES]) {
    int a_dense_element_ctr = 0;
    int b_dense_element_ctr = 0;
    
    //Send pairs of elements to PEs as tasks
    send_pairs:
    while (a_dense_element_ctr < a_dense_elements) {
        for (int pe = 0; pe < NUM_PES; pe++) {
            if (a_dense_element_ctr < a_dense_elements && 
                task_a_position_streams[pe].size() < task_a_position_streams[pe].capacity()) {
                
                task_a_position_streams[pe].write(a_dense_element_memory[a_dense_element_ctr]);
                task_b_position_streams[pe].write(b_dense_element_memory[b_dense_element_ctr]);

                b_dense_element_ctr++;
                if (b_dense_element_ctr == b_dense_elements) {
                    a_dense_element_ctr++;
                    b_dense_element_ctr = 0;
                }
            }
        }
    }

    //Send last signal to finalize PEs
    for (int pe = 0; pe < NUM_PES; pe++) {
        task_a_position_streams[pe].write({0, 0, 0, 0, 0, true});
        task_b_position_streams[pe].write({0, 0, 0, 0, 0, true});
    }
}