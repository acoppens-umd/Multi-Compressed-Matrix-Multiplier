#ifndef test_bench_util_h
#define test_bench_util_h

#include "types.h"
#include <fstream>
#include <iostream>
#include <string.h>

void load_test_case(const std::string &filename,
                    int &a_num_rows, int &a_num_cols, int &b_num_rows, int &b_num_cols,
                    ap_uint<64> *&a_data, ap_uint<64> *&b_data,
                    ap_uint<32> *&a_meta_data, ap_uint<32> *&b_meta_data,
                    ap_uint<32> *&out_data);

#endif