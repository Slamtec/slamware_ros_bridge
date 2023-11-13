/**
* shm_topic_manager.h
* publish/subscribe mechanism via shared memory
*
* Copyright (c) 2019 Shanghai SlamTec Co., Ltd.
*/
#pragma once
#include "shm_topic.h"
#include <boost/make_shared.hpp>

namespace rpos{ namespace system{ namespace shared_memory{ 

    class ShmTopicManager 
    {
    public:
        ShmTopicManager(ShmTopicMgrData* pData):data_(pData){} 

        bool containsTopic(const std::string& name)
        {
            shared_string_t keyName(name.c_str(),SharedMemory::getInstance()->getAllocator<char>());
            return  data_->topics_.find(keyName) != data_->topics_.end(); 
        }

        template < class PayloadT >
        boost::shared_ptr<ShmSubscription<PayloadT>>  subscribe(const std::string& name, ShmTopicQos qos, const ShmSubscriptionOptions& options)
        {
            auto topic = getOrCreateTopic<PayloadT>(name, qos);
            if (topic)
            {
                return topic->subscribe(options);
            }
            return nullptr;
        }

        template < class PayloadT >
        boost::shared_ptr<ShmTopic<PayloadT>> getOrCreateTopic(const std::string& name, ShmTopicQos qos)
        {
            try
            {
                shared_string_t keyName(name.c_str(),SharedMemory::getInstance()->getAllocator<char>());
                ShmTopicData* pData = nullptr;
                {  
                    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
                    doTimedLock(lock);
                    string_handle_map_t::iterator iter = data_->topics_.find(keyName);
                    if (iter != data_->topics_.end())
                    {
                        pData = SharedMemory::getInstance()->getAddressFromHandle<ShmTopicData>(iter->second); 
                    }
                    else
                    {   
                        shared_memory_ptr mem_ptr = SharedMemory::getInstance()->getMemory();
                        pData = mem_ptr->construct<ShmTopicData>(boost::interprocess::anonymous_instance)(name, qos); 
                        memory_handle_t handle = mem_ptr->get_handle_from_address(pData); 
                        data_->topics_.insert(string_handle_pair_t(keyName,handle)); 
                    } 
                }
                return boost::make_shared<ShmTopic<PayloadT>>(pData);
            }catch(const boost::interprocess::interprocess_exception& ex)
            { 
                shmLogger.error_out("getOrCreateTopic failed:%s",ex.what());
            }
            return nullptr;
        }

        template < class PayloadT >
        void deleteTopic(const std::string& name)
        { 
            ShmTopicData* pData = nullptr;
            try
            {
                shared_string_t keyName(name.c_str(),SharedMemory::getInstance()->getAllocator<char>());
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
                doTimedLock(lock);
                string_handle_map_t::iterator iter = data_->topics_.find(keyName); 
                if (iter != data_->topics_.end())
                {
                    pData = SharedMemory::getInstance()->getAddressFromHandle<ShmTopicData>(iter->second);  
                    data_->topics_.erase(iter);
                } 
            } 
            catch (const boost::interprocess::interprocess_exception& e)
            {
                shmLogger.error_out("delete topic error:%s",e.what());
            }
            if (pData != nullptr)
            {
                boost::shared_ptr<ShmTopic<PayloadT>> shmTopic = boost::make_shared<ShmTopic<PayloadT>>(pData);
                shmTopic->destroy(); 
            }
        }    

        template < class PayloadT >
        void deleteTopic(boost::shared_ptr<ShmTopic<PayloadT>>& shmTopic)
        {
            try
            {
                shared_string_t keyName(shmTopic->getName().c_str(),SharedMemory::getInstance()->getAllocator<char>());   
                shmTopic->destroy();  
                boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(data_->mutex_, boost::interprocess::defer_lock);
                doTimedLock(lock);
                data_->topics_.erase(keyName);    
            } 
            catch (const boost::interprocess::interprocess_exception& e)
            {
                shmLogger.error_out("delete topic error:%s",e.what());
            }
        } 
    private:
        void doTimedLock(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>& lock)
        {
            boost::system_time timeout = boost::get_system_time() + boost::posix_time::seconds(3);
            while (!lock.timed_lock(timeout))
            {
                timestamp_t ts = rpos::system::util::high_resolution_clock::get_time_in_ms();
                if ((ts - data_->lastLockTime_) > c_lockTimeoutInMS)
                {
                    shmLogger.error_out("lock owner hold it for too long time, release the topicManager lock.");
                    data_->mutex_.unlock(); 
                }
                timeout = boost::get_system_time() + boost::posix_time::seconds(1);
            } 
            data_->lastLockTime_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
        }
    private:
        ShmTopicMgrData * data_; 
    }; 
}}}