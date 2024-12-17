#include "test_bench_util.h"

void load_test_case(const std::string &filename,
                    int &a_num_rows, int &a_num_cols, int &b_num_rows, int &b_num_cols,
                    int &a_num_dense_diags, int &a_num_dense_blocks, 
                    int &b_num_dense_diags, int &b_num_dense_blocks,
                    ap_uint<128> *&a_data, ap_uint<128> *&b_data,
                    ap_uint<32> *&a_meta_data, ap_uint<32> *&b_meta_data,
                    ap_uint<32> *&out_data) {
    std::fstream fin;
    fin.open(filename, std::ios::in);

    //Load A statistics
    std::string a_num_rows_str;
    std::getline(fin, a_num_rows_str, ',');
    a_num_rows = std::stoi(a_num_rows_str);
    std::string a_num_cols_str;
    std::getline(fin, a_num_cols_str, ',');
    a_num_cols = std::stoi(a_num_cols_str);
    std::string a_num_dense_diags_str;
    std::getline(fin, a_num_dense_diags_str, ',');
    a_num_dense_diags = std::stoi(a_num_dense_diags_str);
    std::string a_num_dense_blocks_str;
    std::getline(fin, a_num_dense_blocks_str);
    a_num_dense_blocks = std::stoi(a_num_dense_blocks_str);

    //Load a meta data
    std::string a_meta_data_line;
    std::getline(fin, a_meta_data_line);

    std::stringstream a_meta_data_line_count_stream(a_meta_data_line);
    int a_meta_data_entries = 0;

    std::string t;
    while(getline(a_meta_data_line_count_stream, t, ',')) {
        a_meta_data_entries++;
    }

    a_meta_data = new ap_uint<32>[a_meta_data_entries];

    std::stringstream a_meta_data_line_get_stream(a_meta_data_line);

    int a_meta_data_entry = 0;
    while(getline(a_meta_data_line_get_stream, t, ',')) {
        a_meta_data[a_meta_data_entry++] = std::stoi(t);
    }

    //Load a data
    std::string a_data_line;
    std::getline(fin, a_data_line);

    std::stringstream a_data_line_count_stream(a_data_line);
    int a_data_entries = 0;

    while(getline(a_data_line_count_stream, t, ',')) {
        a_data_entries++;
    }

    a_data = new ap_uint<128>[(a_data_entries + 3) / 4];

    std::stringstream a_data_line_get_stream(a_data_line);

    int a_data_entry = 0;
    while(getline(a_data_line_get_stream, t, ',')) {
        bit_float bf;
        bf.as_float32 = std::stof(t);
        a_data[a_data_entry / 4](32 * (a_data_entry % 4 + 1) - 1, 32 * (a_data_entry % 4)) = bf.as_uint32;
        a_data_entry++;
    }

    //Load B statistics
    std::string b_num_rows_str;
    std::getline(fin, b_num_rows_str, ',');
    b_num_rows = std::stoi(b_num_rows_str);
    std::string b_num_cols_str;
    std::getline(fin, b_num_cols_str, ',');
    b_num_cols = std::stoi(b_num_cols_str);
    std::string b_num_dense_diags_str;
    std::getline(fin, b_num_dense_diags_str, ',');
    b_num_dense_diags = std::stoi(b_num_dense_diags_str);
    std::string b_num_dense_blocks_str;
    std::getline(fin, b_num_dense_blocks_str);
    b_num_dense_blocks = std::stoi(b_num_dense_blocks_str);

    //Load b meta data
    std::string b_meta_data_line;
    std::getline(fin, b_meta_data_line);

    std::stringstream b_meta_data_line_count_stream(b_meta_data_line);
    int b_meta_data_entries = 0;

    while(getline(b_meta_data_line_count_stream, t, ',')) {
        b_meta_data_entries++;
    }

    b_meta_data = new ap_uint<32>[b_meta_data_entries];

    std::stringstream b_meta_data_line_get_stream(b_meta_data_line);

    int b_meta_data_entry = 0;
    while(getline(b_meta_data_line_get_stream, t, ',')) {
        b_meta_data[b_meta_data_entry++] = std::stoi(t);
    }

    //Load a data
    std::string b_data_line;
    std::getline(fin, b_data_line);

    std::stringstream b_data_line_count_stream(b_data_line);
    int b_data_entries = 0;

    while(getline(b_data_line_count_stream, t, ',')) {
        b_data_entries++;
    }

    b_data = new ap_uint<128>[(b_data_entries + 3) / 4];

    std::stringstream b_data_line_get_stream(b_data_line);

    int b_data_entry = 0;
    while(getline(b_data_line_get_stream, t, ',')) {
        bit_float bf;
        bf.as_float32 = std::stof(t);
        b_data[b_data_entry / 4](32 * (b_data_entry % 4 + 1) - 1, 32 * (b_data_entry % 4)) = bf.as_uint32;
        b_data_entry++;
    }

    fin.close();

    out_data = new ap_uint<32>[a_num_rows * b_num_cols];
    bit_float zero;
    zero.as_float32 = 0.0f;

    for (int i = 0; i < a_num_rows * b_num_cols; i++) {
        out_data[i] = zero.as_uint32;
    }
}