#pragma once
#include "shm_message.h"
#include <rpos/message/message.h>
#include <rpos/system/util/util.h>

#include <boost/thread.hpp> 

namespace rpos{ namespace system{ namespace shared_memory{ 
    
    template < class PayloadT >
    class ShmTopic;
    
    #define SUBSCRIPTION_TIMEOUT_INFINITE (-1)

    enum ShmSubscriptionQos
    {
        ShmSubscriptionQosAtMostOnce = 0, //read latest message
        ShmSubscriptionQosAll //read all message
    };

    struct ShmSubscriptionOptions
    {
        ShmSubscriptionQos qos_;
        int timeoutInSeconds_;
        ShmSubscriptionOptions()
            :qos_(ShmSubscriptionQosAtMostOnce)
            ,timeoutInSeconds_(30)
        {}
    };

    struct ShmSubscriptionData
    {
        ShmSubscriptionOptions options_;
        message_seq_num_t latestMsgSeqNum_;
        timestamp_t lastUpdate_;
        ShmSubscriptionData():lastUpdate_(rpos::system::util::high_resolution_clock::get_time_in_ms())
        {

        }
    };

    template < class PayloadT >
    class ShmSubscription 
    { 
    public:
        ShmSubscription(memory_handle_t handle,const boost::shared_ptr<ShmTopic<PayloadT>>& topic)
            : subHandle(handle)
            , topic_(topic)
        {}
        ~ShmSubscription()
        {
            unsubscribe();
        }

        bool read(rpos::message::Message<PayloadT>& outResult)
        { 
            if (!topic_)
                return false; 

            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(topic_->getMutex_(), boost::interprocess::defer_lock);
            if(!topic_->doTimedLock_(lock))
            {
                return false; 
            }

            return topic_->read_(subHandle, outResult);
        }

        bool tryRead(rpos::message::Message<PayloadT>& outResult)
        { 
            if (!topic_)
                return false; 

            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(topic_->getMutex_(), boost::interprocess::defer_lock);
            if (!topic_->doTryLock_(lock))
            {
                return false;
            }

            return topic_->read_(subHandle, outResult);
        }

        bool readAll(std::vector<rpos::message::Message<PayloadT>>& outResult)
        {
            outResult.clear();
            if (!topic_)
                return false;

            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(topic_->getMutex_(), boost::interprocess::defer_lock);
            if(!topic_->doTimedLock_(lock))
            {
                return false; 
            }
            return topic_->readAll_(subHandle, outResult);
        }

        void unsubscribe()
        {
            if (!topic_)
                return; 
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(topic_->getMutex_(), boost::interprocess::defer_lock);
            if(topic_->doTimedLock_(lock))
            {
                topic_->unsubscribe_(subHandle);
            }
            topic_ = nullptr;
        }
    private:
        memory_handle_t subHandle;
        boost::shared_ptr<ShmTopic<PayloadT>> topic_;
    }; 

}}}