#ifndef util_h
#define util_h

#include "kernel.h"
#include "types.h"

inline void min(ap_uint<32> &val1, ap_uint<32> &val2, ap_uint<32> &out) {
    if (val1 > val2) {
        out = val2;
    } else {
        out = val1;
    }
}


inline void max(ap_uint<32> &val1, ap_uint<32> &val2, ap_uint<32> &out) {
    if (val1 < val2) {
        out = val2;
    } else {
        out = val1;
    }
}

#endif