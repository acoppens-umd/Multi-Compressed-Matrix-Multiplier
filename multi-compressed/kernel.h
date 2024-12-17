#ifndef kernel_h
#define kernel_h

#include "ap_axi_sdata.h"
#include "ap_int.h"
#include <hls_stream.h>

#include "types.h"

void kernel(const int a_num_rows,
            const int a_num_cols,
            const int b_num_rows,
            const int b_num_cols,
            const int a_num_dense_diags,
            const int a_num_dense_blocks,
            const int b_num_dense_diags,
            const int b_num_dense_blocks,
            const int a_dense_diag_start_addr,
            const int b_dense_diag_start_addr,
            const int a_dense_block_start_addr,
            const int b_dense_block_start_addr,    
            
            const a_in_port_t *a_in_port,
            const b_in_port_t *b_in_port,

            const a_meta_in_port_t *a_meta_in_port,
            const b_meta_in_port_t *b_meta_in_port,
            
            out_port_t *out_port,
            
            int &write_back_counter,
            int &atomic_write_counter);

#endif