#pragma once
#include "shm_defs.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

namespace rpos{ namespace system{ namespace shared_memory{

    class ShmHeartbeat
    {
    public:
        ShmHeartbeat();
        ~ShmHeartbeat();
        void start(bool isOwner, boost::uint32_t*);
        void stop();
        bool isActive(){return bHeartbeatActive_;}
    private:
        void worker();
    private: 
        bool isOwner_;
        boost::atomic_bool working_;
        boost::uint32_t* pHeartbeatValue_;
        bool bHeartbeatActive_;
        boost::thread  thread_; 
    };

}}}