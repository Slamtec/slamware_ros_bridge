/**
* shared_memory.h
* Shared memory manager
*
* Copyright (c) 2019 Shanghai SlamTec Co., Ltd.
*/

#pragma once
#include "shm_defs.h"
#include "shm_heartbeat.h"
#include <rpos/system/util/log.h>

namespace rpos{ namespace system{ namespace shared_memory{ 

    extern RPOS_CORE_API rpos::system::util::LogScope shmLogger;

    struct ShmTopicMgrData  
    {
        ShmTopicMgrData(const default_allocator_t &alloc)
            :topics_(alloc),lastLockTime_(0)
        {}
        string_handle_map_t topics_; 
        boost::interprocess::interprocess_mutex mutex_;
        timestamp_t lastLockTime_;
    };
    class ShmTopicManager;

    class RPOS_CORE_API SharedMemory
    {
    public:
        SharedMemory();
        ~SharedMemory();
        static boost::shared_ptr<SharedMemory> getInstance();
        bool init(MemoryAccessRight access, std::string name = "", int size = 0);
        void destroy();
        bool isValid() { return (status_ == SharedMemoryStatusSuccess) && heartbeat_.isActive();}
        bool makeItValid(SharedMemoryStatus* memStatus);
        bool reopen();
        
        default_allocator_t getDefaultAllocator();

        template < class T> 
        boost::interprocess::allocator<T, segment_manager_t> getAllocator()
        { 
            return boost::interprocess::allocator<T, segment_manager_t>(memory_->get_segment_manager());   
        }

        shared_memory_ptr getMemory(); 

        memory_handle_t getHandleFromAddress(void *ptr);

        template < class PayloadT>
        PayloadT* getAddressFromHandle(memory_handle_t handle)
        {
            return (PayloadT*)memory_->get_address_from_handle(handle);
        } 

        //create an anonymous object on shared memory
        //return nullptr if failed, otherwise return pointer
        //@param handle, if succeeded, return handle of object
        template < class PayloadT>
        PayloadT* createAnonymousObject(memory_handle_t& handle)
        {
            try
            {
                PayloadT* ptr = memory_->construct<PayloadT>(boost::interprocess::anonymous_instance)();  
                handle = memory_->get_handle_from_address(ptr);
                return ptr;
            }catch (const boost::interprocess::bad_alloc& ex) 
            { 
                shmLogger.error_out("create anonymous object error:%s, exception: %s", typeid(PayloadT).name(), ex.what()); 
            }
            return nullptr;
        }

        template < class PayloadT>
        void destroyAnonymousObject(memory_handle_t handle)
        {
            PayloadT *ptr = getAddressFromHandle<PayloadT>(handle);
            if (ptr)
            {
                memory_->destroy_ptr<PayloadT>(ptr);
            }
        }

        template < class PayloadT>
        void destroyPtr(PayloadT* ptr)
        {
            memory_->destroy_ptr<PayloadT>(ptr); 
        }

        boost::shared_ptr<ShmTopicManager> getTopicManager();
    private:
        bool isOwner(){ return MemoryAccessRightOwner == access_ || MemoryAccessRightVolatileOwner == access_ ;}
    private: 
        MemoryAccessRight access_;
        SharedMemoryStatus status_;

        shared_memory_ptr memory_;
        shm_object_ptr memoryHeader_;
        mapped_region_ptr headerRegion_;

        ShmHeartbeat heartbeat_;  
        ShmTopicMgrData *topicMgr_;
        std::string memName_;
        std::string memHeaderName_;
        int memSize_;
    }; 

}}}