#include "processing_element.h"
#include "util.h"

void processing_element(
    //in
    hls::stream<TaskAPosition_t, DEFAULT_STREAM_DEPTH> &task_a_position_stream, //from job distributor
    hls::stream<TaskBPosition_t, DEFAULT_STREAM_DEPTH> &task_b_position_stream,
    
    //out
    hls::stream<AInPortReadReq_t, DEFAULT_STREAM_DEPTH> &a_read_addr_stream,
    hls::stream<BInPortReadReq_t, DEFAULT_STREAM_DEPTH> &b_read_addr_stream,

    hls::stream<AtomicInstruction_t, DEFAULT_STREAM_DEPTH> &instruction_stream
) {
    bool last_task = false;

    compute_loop: do {
        TaskAPosition_t task_a_position = task_a_position_stream.read();
        TaskBPosition_t task_b_position = task_b_position_stream.read();

        if (task_a_position.end) {
            last_task = true;
        } else if (task_a_position.start_indptr == task_a_position.end_indptr || 
                   task_b_position.start_indptr == task_b_position.end_indptr) {
            //Skip
        } else {
            a_read_addr_stream.write({task_a_position.start_indptr, task_a_position.end_indptr, false});
            b_read_addr_stream.write({task_b_position.start_indptr, task_b_position.end_indptr, false});
            instruction_stream.write({
                task_a_position.row, task_b_position.col,
                task_a_position.start_indptr, task_a_position.end_indptr,
                task_b_position.start_indptr, task_b_position.end_indptr,
                false
            });
        }
    } while (!last_task);

    a_read_addr_stream.write({0, 0, true});
    b_read_addr_stream.write({0, 0, true});

    instruction_stream.write({0, 0, 0, 0, 0, 0, true});
}