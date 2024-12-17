#include "kernel.h"
#include "task_distributor.h"
#include "read_controller.h"
#include "processing_element.h"
#include "processing_element_multiplier.h"
#include "write_back_gate.h"
#include "write_back_network.h"
#include "types.h"


void kernel(//in
            const int a_num_rows,
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

            out_port_t *out_port,     //Output Result

            int &write_back_counter,
            int &atomic_write_counter
	)
{
#pragma HLS cache port=a_in_port lines=16384 depth=128 
#pragma HLS cache port=b_in_port lines=16384 depth=128

#pragma HLS cache port=a_meta_in_port lines=1 depth=32
#pragma HLS cache port=b_meta_in_port lines=1 depth=32
    
#pragma HLS INTERFACE m_axi port=a_in_port depth = 1024 bundle=ch0a
#pragma HLS INTERFACE m_axi port=b_in_port depth = 1024 bundle=ch0b

#pragma HLS INTERFACE m_axi port=a_meta_in_port depth = 1024 bundle=ch1a
#pragma HLS INTERFACE m_axi port=b_meta_in_port depth = 1024 bundle=ch1b

#pragma HLS INTERFACE m_axi port=out_port depth = 16384 bundle=OUT_BUS 

#pragma HLS DATAFLOW

    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> task_a_position_streams[NUM_PES]; //from task distributor to PEs
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> task_b_position_streams[NUM_PES];

    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> a_read_addr_streams[NUM_PES]; //from PEs to read controller
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> b_read_addr_streams[NUM_PES];

    hls::stream<a_in_port_t, DEFAULT_STREAM_DEPTH> a_data_streams[NUM_PES]; //from read controller to PEMs
    hls::stream<b_in_port_t, DEFAULT_STREAM_DEPTH> b_data_streams[NUM_PES];

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> instruction_streams[NUM_PES]; //from PEs to PEMs

    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> out_streams[NUM_PES]; //from PEs to Write Back Gate

    hls::stream<bool, DEFAULT_STREAM_DEPTH> gate_to_network_initiation_stream; //from Write Back Gate to Write Back Network
    hls::stream<DenseVal_t, DEFAULT_STREAM_DEPTH> gate_to_network_streams[NUM_PES];

    task_distributor(a_num_dense_diags, a_num_dense_blocks, b_num_dense_diags, b_num_dense_blocks,
                     a_dense_diag_start_addr, b_dense_diag_start_addr,
                     a_dense_block_start_addr, b_dense_block_start_addr, 
                     a_meta_in_port, b_meta_in_port,
                     task_a_position_streams, task_b_position_streams);

    for (int pe = 0; pe < NUM_PES; pe++) {
        #pragma HLS UNROLL
        processing_element(pe, task_a_position_streams[pe], task_b_position_streams[pe],
                           a_num_rows, a_num_cols, b_num_rows, b_num_cols,
                           a_read_addr_streams[pe], b_read_addr_streams[pe],
                           instruction_streams[pe]);
    }

    read_controller(a_in_port, b_in_port, 
                    a_read_addr_streams, b_read_addr_streams,
                    a_data_streams, b_data_streams);

    for (int pe = 0; pe < NUM_PES; pe++) {
        #pragma HLS UNROLL
        processing_element_multiplier(a_data_streams[pe], b_data_streams[pe],
                                      instruction_streams[pe], out_streams[pe]);
    }
    
    write_back_gate(out_streams, gate_to_network_initiation_stream,
                    gate_to_network_streams);

    write_back_network(a_num_rows, b_num_cols, 
                       gate_to_network_initiation_stream, gate_to_network_streams, 
                       out_port, write_back_counter, atomic_write_counter);
}