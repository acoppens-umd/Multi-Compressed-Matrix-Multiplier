#include "test_bench_util.h"

void load_test_case(const std::string &filename,
                    int &a_num_rows, int &a_num_cols, int &b_num_rows, int &b_num_cols,
                    ap_uint<64> *&a_data, ap_uint<64> *&b_data,
                    ap_uint<32> *&a_meta_data, ap_uint<32> *&b_meta_data,
                    ap_uint<32> *&out_data) {
    std::fstream fin;
    fin.open(filename, std::ios::in);

    //Load a statistics
    std::string a_num_rows_str;
    std::getline(fin, a_num_rows_str, ',');
    a_num_rows = std::stoi(a_num_rows_str);
    std::string a_num_cols_str;
    std::getline(fin, a_num_cols_str);
    a_num_cols = std::stoi(a_num_cols_str);

    //Load a meta data
    std::string a_meta_data_line;
    std::getline(fin, a_meta_data_line);

    std::string t;

    std::stringstream a_meta_data_line_stream(a_meta_data_line);
    a_meta_data = new ap_uint<32>[a_num_rows + 1];

    int a_meta_data_entry = 0;
    while(getline(a_meta_data_line_stream, t, ',')) {
        a_meta_data[a_meta_data_entry] = std::stoi(t);

        a_meta_data_entry++;
    }

    //Load a data
    int a_num_data = a_meta_data[a_num_rows];
    a_data = new ap_uint<64>[a_num_data];

    std::string a_data_line;
    std::string a_index_line;
    std::getline(fin, a_data_line);
    std::getline(fin, a_index_line);

    std::stringstream a_data_line_stream(a_data_line);
    std::stringstream a_index_line_stream(a_index_line);

    int a_data_entry = 0;
    while(getline(a_data_line_stream, t, ',')) {
        std::string s;
        getline(a_index_line_stream, s, ',');
        bit_float bf;
        bf.as_float32 = std::stof(t);
        a_data[a_data_entry](31, 0) = bf.as_uint32;
        a_data[a_data_entry](63, 32) = std::stoi(s);

        a_data_entry++;
    }

    //Load b statistics
    std::string b_num_rows_str;
    std::getline(fin, b_num_rows_str, ',');
    b_num_rows = std::stoi(b_num_rows_str);
    std::string b_num_cols_str;
    std::getline(fin, b_num_cols_str);
    b_num_cols = std::stoi(b_num_cols_str);

    //Load b meta data
    std::string b_meta_data_line;
    std::getline(fin, b_meta_data_line);

    std::stringstream b_meta_data_line_stream(b_meta_data_line);
    b_meta_data = new ap_uint<32>[b_num_cols + 1];

    int b_meta_data_entry = 0;
    while(getline(b_meta_data_line_stream, t, ',')) {
        b_meta_data[b_meta_data_entry] = std::stoi(t);

        b_meta_data_entry++;
    }

    //Load b data
    int b_num_data = b_meta_data[b_num_cols];
    b_data = new ap_uint<64>[b_num_data];

    std::string b_data_line;
    std::string b_index_line;
    std::getline(fin, b_data_line);
    std::getline(fin, b_index_line);

    std::stringstream b_data_line_stream(b_data_line);
    std::stringstream b_index_line_stream(b_index_line);

    int b_data_entry = 0;
    while(getline(b_data_line_stream, t, ',')) {
        std::string s;
        getline(b_index_line_stream, s, ',');
        bit_float bf;
        bf.as_float32 = std::stof(t);
        b_data[b_data_entry](31, 0) = bf.as_uint32;
        b_data[b_data_entry](63, 32) = std::stoi(s);

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