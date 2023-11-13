#pragma once
#include "shm_subscription.h"
#include "shared_memory.h"
#include <rpos/system/util/util.h>
  
#include <boost/make_shared.hpp>
#include <boost/thread/thread_time.hpp>

namespace rpos{ namespace system{ namespace shared_memory{
    
    enum ShmTopicQos
    {
        ShmTopicQosMessageSingleton = 0,
        ShmTopicQosMessageQueue
    };    
    
    struct ShmTopicData
    {   
        explicit ShmTopicData(const std::string& name, ShmTopicQos qos)
            : name_(name.c_str(),SharedMemory::getInstance()->getAllocator<char>())
            , qos_(qos)
            , disposing_(false)
            , subscriptions_(SharedMemory::getInstance()->getAllocator<memory_handle_t>())
            , messages_(SharedMemory::getInstance()->getAllocator<MessageEntry>())
            , lastGeneratedMsgSeqNum_(0)
            , lastLockTime_(0)
            , recycleBin_(SharedMemory::getInstance()->getAllocator<memory_handle_t>())
        {}
        shared_string_t name_;
        ShmTopicQos qos_;
        bool disposing_;

        handle_set_t subscriptions_;
        message_deque_t messages_;
        message_seq_num_t lastGeneratedMsgSeqNum_; 
        timestamp_t lastLockTime_; 
        boost::interprocess::interprocess_mutex mutex_; 
        handle_list_t recycleBin_;
    };
    
    struct DbowFrame
    {
        std::uint64_t timeStamp;
        int rows;
        int cols;
        int channels;
        rpos::system::shared_memory::buffer_vector_t image;
        DbowFrame() : timeStamp(0), rows(0), cols(0), channels(0), image(SharedMemory::getInstance()->getAllocator<uint8_t>())
        {}
    };

    struct LaserPoint
    {
        float angle;
        float dist;
        bool valid;
        shared_string_t layer;
        LaserPoint() : angle(0.f), dist(0.f), valid(false), layer(SharedMemory::getInstance()->getAllocator<char>())
        {}
    };

    struct LaserScan
    {
        rpos::system::shared_memory::laser_point_vector_t data;
        LaserScan() : data(SharedMemory::getInstance()->getAllocator<LaserPoint>())
        {}
    };

    template < class PayloadT >
    class ShmTopic : public boost::enable_shared_from_this<ShmTopic<PayloadT>>
    {
        static const uint32_t c_maxMessageCount = 10000; 
    public:
        explicit ShmTopic(ShmTopicData* data)
            : data_(data)
        { 
            BOOST_ASSERT(data_);
            maxMessageTrashSize_ = std::max<uint32_t>(100*1024/sizeof(PayloadT),10);
        }
        ~ShmTopic()
        {  
        }
        std::string getName(){return std::string(data_->name_.c_str());}

        boost::shared_ptr<ShmSubscription<PayloadT>> subscribe(const ShmSubscriptionOptions& options)
        {   
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
            if(!doTimedLock_(lock))
            {
                return nullptr;
            }

            if (data_->disposing_)
            {
                return nullptr;
            }
            memory_handle_t handle;
            ShmSubscriptionData* pData = SharedMemory::getInstance()->createAnonymousObject<ShmSubscriptionData>(handle);
            if (pData == nullptr)
            {
                return nullptr;
            }
            pData->options_ = options;
            pData->latestMsgSeqNum_ = data_->lastGeneratedMsgSeqNum_;
            data_->subscriptions_.insert(handle); 

            return boost::make_shared<ShmSubscription<PayloadT>>(handle, this->shared_from_this());
        }
        bool hasSubscription(){ return !data_->subscriptions_.empty();}
        bool publish(const PayloadT& payload)
        {
            return doPublish_(payload, INVALID_TIMESTAMP);
        }
        bool publish(const rpos::message::Message<PayloadT>& msg)
        {
            return doPublish_(msg.payload, msg.timestamp);
        }
       
    protected:
        friend class ShmSubscription<PayloadT>;
        friend class ShmTopicManager;
        void destroy()
        {
            data_->disposing_ = true;
            ShmTopicData * pData = data_;
            {
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
                if(!doTimedLock_(lock))
                {
                    return;
                }
                for (handle_set_t::iterator iter = data_->subscriptions_.begin(); iter != data_->subscriptions_.end(); ++iter)
                {
                    SharedMemory::getInstance()->destroyAnonymousObject<ShmSubscriptionData>(*iter);  
                }
                data_->subscriptions_.clear();
                destroyMessages_();
                data_ = NULL;
            }
            SharedMemory::getInstance()->destroyPtr<ShmTopicData>(pData); 
        } 
        boost::interprocess::interprocess_mutex& getMutex_(){return data_->mutex_;} 
        bool read_(memory_handle_t subHandle, rpos::message::Message<PayloadT>& message)
        {
            ShmSubscriptionData *pSub = findSubscription_(subHandle);
            if (pSub == nullptr)
            {
                shmLogger.debug_out("invalid subscription handle");
                return false;
            }
            bool readResult = false; 
            if (pSub->options_.qos_ == ShmSubscriptionQosAtMostOnce)
            {
                if (!data_->messages_.empty())
                {
                    MessageEntry & msg = data_->messages_.back();
                    if (pSub->latestMsgSeqNum_ != msg.seqNum)
                    {
                        PayloadT* ptr = SharedMemory::getInstance()->getAddressFromHandle<PayloadT>(msg.handle); 
                        message.payload = *ptr;
                        message.timestamp = msg.timestamp;
                        pSub->latestMsgSeqNum_ = msg.seqNum;
                        readResult = true;
                    }
                } 
            }
            else if (pSub->options_.qos_ == ShmSubscriptionQosAll)
            {
                message_deque_t::iterator iter = lookupNextMessage_(pSub->latestMsgSeqNum_); 
                if (iter != data_->messages_.end())
                {
                    PayloadT* ptr = SharedMemory::getInstance()->getAddressFromHandle<PayloadT>(iter->handle); 
                    message.payload = *ptr;
                    message.timestamp = iter->timestamp;
                    pSub->latestMsgSeqNum_ = iter->seqNum;
                    readResult = true;
                }
            }

            if (readResult && data_->qos_ == ShmTopicQosMessageQueue)
                retireMessages_();
            pSub->lastUpdate_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
            return readResult;
        }

        bool readAll_(memory_handle_t subHandle, std::vector<rpos::message::Message<PayloadT>>& outResult)
        {     
            ShmSubscriptionData *pSub = findSubscription_(subHandle);
            if (pSub == nullptr)
            {
                shmLogger.debug_out("invalid subscription handle");
                return false;
            }
            auto iter = lookupNextMessage_(pSub->latestMsgSeqNum_);

            for (; iter != data_->messages_.end(); ++iter)
            {
                PayloadT* ptr =  SharedMemory::getInstance()->getAddressFromHandle<PayloadT>(iter->handle);  
                outResult.push_back(rpos::message::Message<PayloadT>(*ptr));
                outResult.back().timestamp = iter->timestamp;
            }

            bool bRet = !outResult.empty();
            if (bRet)
            {
                pSub->latestMsgSeqNum_ = data_->messages_.rbegin()->seqNum;
                if(data_->qos_ == ShmTopicQosMessageQueue)  
                    retireMessages_();
            } 
            pSub->lastUpdate_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
            return bRet;
        }

        void unsubscribe_(memory_handle_t subHandle)
        {
            if (retireSubscription_(subHandle) && data_->qos_ == ShmTopicQosMessageQueue)
            {
                retireMessages_(); 
            } 
        }

    private: 
        bool doPublish_(const PayloadT& payload, timestamp_t msgTime)
        {           
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
            if(!doTimedLock_(lock))
            {
                return false;
            }

            timestamp_t ts = (INVALID_TIMESTAMP == msgTime) ? rpos::system::util::high_resolution_clock::get_time_in_ms() : msgTime;
            message_seq_num_t seqNum = ++(data_->lastGeneratedMsgSeqNum_);
            if (data_->qos_ == ShmTopicQosMessageSingleton && !data_->messages_.empty())
            { 
                MessageEntry& msg = data_->messages_.front();
                PayloadT* ptr = SharedMemory::getInstance()->getAddressFromHandle<PayloadT>(msg.handle);  
                *ptr = payload;
                msg.seqNum = seqNum;
                msg.timestamp = ts;
                retireSubscriptions_();
                return true;
            }

            memory_handle_t handle;
            if (data_->recycleBin_.empty())
            {
                PayloadT* ptr = SharedMemory::getInstance()->createAnonymousObject<PayloadT>(handle); 
                if (ptr == nullptr)
                {
                    return false;
                }
                *ptr = payload;
            }  
            else
            {
                handle = data_->recycleBin_.back();
                data_->recycleBin_.pop_back();
                PayloadT* ptr = SharedMemory::getInstance()->getAddressFromHandle<PayloadT>(handle);
                *ptr = payload;
            }
            while (data_->messages_.size() >= c_maxMessageCount)
            {
                retireFirstMessage_();
            }
            data_->messages_.push_back(MessageEntry(handle,seqNum,ts));   
            retireSubscriptions_();
            return true;
        } 
        typename message_deque_t::iterator lookupNextMessage_(message_seq_num_t lastGottenSeqNum)
        {
            message_seq_num_t startSeqNum = lastGottenSeqNum+1;
            const auto itEnd = data_->messages_.end();
            const size_t msgCnt = data_->messages_.size();
            if (msgCnt < 1)
                return itEnd;
            auto itFirst = data_->messages_.begin();
            auto itLast = itFirst + (msgCnt - 1);
            const auto firstSeqNum = itFirst->seqNum;
            const auto lastSeqNum = itLast->seqNum;

            if (startSeqNum <= firstSeqNum)
                return itFirst;
            if (startSeqNum > lastSeqNum)
                return itEnd;

            const auto tDiff = startSeqNum - firstSeqNum;
            BOOST_ASSERT(static_cast<size_t>(tDiff) < msgCnt);
            auto itRes = itFirst + static_cast<size_t>(tDiff);
            BOOST_ASSERT(startSeqNum == itRes->seqNum);
            return itRes;
        }
        bool retireSubscription_(memory_handle_t subHandle)
        {
            handle_set_t::iterator iter = data_->subscriptions_.find(subHandle);
            if (iter == data_->subscriptions_.end())
            {
                return false;
            }
            data_->subscriptions_.erase(iter); 
            SharedMemory::getInstance()->destroyAnonymousObject<ShmSubscriptionData>(subHandle);  
            return true;        
        }
        void retireMessages_()
        {          
            message_seq_num_t retiredMsgSeqNum = data_->lastGeneratedMsgSeqNum_;
            for (auto subIter = data_->subscriptions_.begin(); subIter != data_->subscriptions_.end(); ++subIter)
            {
                ShmSubscriptionData* pSub = SharedMemory::getInstance()->getAddressFromHandle<ShmSubscriptionData>(*subIter); 
                if (pSub != nullptr)
                {
                    retiredMsgSeqNum = std::min<int32_t>(retiredMsgSeqNum, pSub->latestMsgSeqNum_);
                }
            }
             
            while (!data_->messages_.empty() && retiredMsgSeqNum >= data_->messages_.front().seqNum)
            {
                retireFirstMessage_();
            }
        }
        void retireSubscriptions_()
        {
            if (data_->subscriptions_.empty())
                return;

            bool subscriptionRemoved_ = false;
            timestamp_t now = rpos::system::util::high_resolution_clock::get_time_in_ms();
            handle_set_t::iterator iter = data_->subscriptions_.begin();
            while (iter != data_->subscriptions_.end())
            {
                ShmSubscriptionData* pSub = SharedMemory::getInstance()->getAddressFromHandle<ShmSubscriptionData>(*iter);
                if (pSub == nullptr)
                {
                    shmLogger.error_out("Inconsistency of subscription");
                    iter = data_->subscriptions_.erase(iter); 
                    continue;
                }

                if (pSub->options_.timeoutInSeconds_ != SUBSCRIPTION_TIMEOUT_INFINITE 
                    && (now - pSub->lastUpdate_) > (pSub->options_.timeoutInSeconds_*1000))
                {
                    iter = data_->subscriptions_.erase(iter); 
                    SharedMemory::getInstance()->destroyPtr<ShmSubscriptionData>(pSub);  
                    subscriptionRemoved_ = true; 
                    shmLogger.warn_out("subscription timeout, remove it.");
                }
                else
                {
                    ++iter;  
                }
            } 
            if (subscriptionRemoved_ && data_->qos_ == ShmTopicQosMessageQueue)
                retireMessages_();
        }
        ShmSubscriptionData* findSubscription_(memory_handle_t handle)
        {
            if (data_->subscriptions_.find(handle) == data_->subscriptions_.end())
            {
                return nullptr;
            }
            return SharedMemory::getInstance()->getAddressFromHandle<ShmSubscriptionData>(handle);
        }
        bool doTimedLock_(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>& lock)
        {
            bool bRet = true;
            try
            {
                boost::system_time timeout = boost::get_system_time() + boost::posix_time::seconds(1);
                bool isSecondTry = false;
                while (!lock.timed_lock(timeout))
                {
                    timestamp_t ts = rpos::system::util::high_resolution_clock::get_time_in_ms();
                    uint32_t lockTime = uint32_t(ts - data_->lastLockTime_);
                    if (lockTime > c_lockTimeoutInMS)
                    {
                        if (isSecondTry)
                        {
                            shmLogger.warn_out("force to release lock failed");
                            return false;
                        }
                        shmLogger.warn_out("locked for too long time, force to release it[doTimedLock]:%s, %d ms",data_->name_.c_str(), lockTime);
                        data_->mutex_.unlock(); 
                        isSecondTry = true; 
                    }
                    timeout = boost::get_system_time() + boost::posix_time::seconds(1);
                } 
                data_->lastLockTime_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
            }catch(std::exception ex)
            {
                shmLogger.warn_out("lock exception:%s",ex.what());
                bRet = false;
            }
            return bRet;
        }

        bool doTryLock_(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>& lock)
        {
            if(lock.try_lock())
            {
                data_->lastLockTime_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
                return true;
            }
            timestamp_t ts = rpos::system::util::high_resolution_clock::get_time_in_ms();
            if ((ts - data_->lastLockTime_) > c_lockTimeoutInMS)
            {
                shmLogger.error_out("locked for too long time, force to release it[doTryLock_]:%s",data_->name_.c_str());
                data_->mutex_.unlock();  
                if(lock.try_lock())
                    data_->lastLockTime_ = ts;
            }
            return lock.owns();
        }
        void retireFirstMessage_()
        {
            if (data_->recycleBin_.size() >= maxMessageTrashSize_)
            {
                SharedMemory::getInstance()->destroyAnonymousObject<PayloadT>(data_->messages_.front().handle); 
            }
            else
            {
                data_->recycleBin_.push_back(data_->messages_.front().handle);
            }
            data_->messages_.pop_front();
        }
        void destroyMessages_()
        {
            for (auto iter = data_->messages_.begin(); iter != data_->messages_.end(); iter++)
            {
                SharedMemory::getInstance()->destroyAnonymousObject<PayloadT>((*iter).handle);  
            }     
            data_->messages_.clear();
            for (auto iter = data_->recycleBin_.begin(); iter != data_->recycleBin_.end(); iter++)
            {
                SharedMemory::getInstance()->destroyAnonymousObject<PayloadT>((*iter));  
            }
            data_->recycleBin_.clear();
        } 
    protected: 
        ShmTopicData *data_;
        uint32_t maxMessageTrashSize_ ;
    };
     
}}}