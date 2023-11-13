#pragma once
#include "shm_defs.h"

#include <boost/thread.hpp>

namespace rpos{ namespace system{ namespace shared_memory{
     
    #define MEMORY_MESSAGE_INVALID_SEQ_NUM ((int32_t)-1)
    #define INVALID_TIMESTAMP (0)
    typedef int32_t message_seq_num_t;

    struct MessageEntry
    {
        explicit MessageEntry(const memory_handle_t& that, message_seq_num_t seq, timestamp_t ts)
            : handle(that)
            , seqNum(seq)
            , timestamp(ts)
        {
        } 
        memory_handle_t handle; 
        message_seq_num_t seqNum; 
        timestamp_t timestamp;
    }; 

    typedef boost::interprocess::deque<MessageEntry,boost::interprocess::allocator<MessageEntry, segment_manager_t>> message_deque_t;
}}}