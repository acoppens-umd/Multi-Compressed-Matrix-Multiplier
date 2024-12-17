#include "kernel.h"
#include "types.h"
#include "test_bench_util.h"
#include <iostream>

int main() {
    std::cout << "Starting Simulation" << std::endl;

    int a_num_rows;
    int a_num_cols;
    int b_num_rows;
    int b_num_cols;

    int a_num_dense_diags;
    int a_num_dense_blocks;
    int b_num_dense_diags;
    int b_num_dense_blocks;    

    ap_uint<32> *a_meta_data;
    ap_uint<128> *a_data;

    ap_uint<32> *b_meta_data;
    ap_uint<128> *b_data;

    ap_uint<32> *out_data;

    load_test_case("data/test6.csv",
                   a_num_rows, a_num_cols, b_num_rows, b_num_cols,
                   a_num_dense_diags, a_num_dense_blocks, 
                   b_num_dense_diags, b_num_dense_blocks,
                   a_data, b_data,
                   a_meta_data, b_meta_data,
                   out_data);

    int write_back_counter = 0;
    int atomic_write_counter = 0;

    kernel(a_num_rows, a_num_cols, b_num_rows, b_num_cols, 
           a_num_dense_diags, a_num_dense_blocks,
           b_num_dense_diags, b_num_dense_blocks,
           0, 0, 0, 0,
           a_data, b_data, 
           a_meta_data, b_meta_data,
           out_data, write_back_counter, atomic_write_counter);

    std::cout << "Write Backs: " << write_back_counter << std::endl;
    std::cout << "Atomic Write Counter: " << atomic_write_counter << std::endl;

    for (int i = 0; i < a_num_rows; i++) {
        for (int k = 0; k < b_num_cols; k++) {
            bit_float val = {};
            val.as_uint32 = out_data[i * b_num_cols + k];

            std::cout << val.as_float32 << " ";
        }

        std::cout << std::endl;
    }
}