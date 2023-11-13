#pragma once
#include <cstdint>
#include <rpos/core/rpos_core_config.h>

#include <boost/shared_ptr.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/containers/vector.hpp> 
#include <boost/interprocess/containers/list.hpp> 
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/containers/map.hpp>

namespace rpos{ namespace system{ namespace shared_memory{

    struct LaserPoint;

    typedef boost::interprocess::managed_shared_memory::segment_manager segment_manager_t;  
    typedef boost::interprocess::managed_shared_memory::handle_t memory_handle_t;  
 
    typedef boost::interprocess::allocator<void, segment_manager_t> default_allocator_t; 
    typedef boost::interprocess::allocator<char, segment_manager_t> char_allocator_t; 
    typedef boost::interprocess::allocator<std::uint8_t, segment_manager_t> uint8_allocator_t; 
    typedef boost::interprocess::allocator<memory_handle_t, segment_manager_t> handle_allocator_t; 
    typedef boost::interprocess::allocator<LaserPoint, segment_manager_t> laser_point_allocator_t;

    typedef boost::interprocess::basic_string<char, std::char_traits<char>, char_allocator_t> shared_string_t; 
    typedef boost::interprocess::set<shared_string_t, std::less<shared_string_t>, boost::interprocess::allocator<shared_string_t, segment_manager_t>> string_set_t;
    typedef boost::interprocess::set<memory_handle_t,std::less<memory_handle_t>,handle_allocator_t> handle_set_t;  
    typedef boost::interprocess::list<memory_handle_t,handle_allocator_t> handle_list_t;
    typedef boost::interprocess::vector<std::uint8_t,uint8_allocator_t> buffer_vector_t;
    typedef boost::interprocess::vector<LaserPoint, laser_point_allocator_t> laser_point_vector_t;


    typedef std::pair<const shared_string_t, memory_handle_t> string_handle_pair_t;
    typedef boost::interprocess::allocator<string_handle_pair_t, segment_manager_t> string_handle_pair_allocator_t;
    typedef boost::interprocess::map<shared_string_t, memory_handle_t, std::less<shared_string_t>, string_handle_pair_allocator_t> string_handle_map_t;

    typedef boost::shared_ptr<boost::interprocess::managed_shared_memory> shared_memory_ptr;
    typedef boost::shared_ptr<boost::interprocess::shared_memory_object> shm_object_ptr;
    typedef boost::shared_ptr<boost::interprocess::mapped_region> mapped_region_ptr;

    typedef int64_t timestamp_t;

    enum MemoryAccessRight
    {
        MemoryAccessRightUser,
        MemoryAccessRightOwner, 
        MemoryAccessRightVolatileOwner //always create new shared memory
    };

    enum SharedMemoryStatus
    {
        SharedMemoryStatusSuccess = 0,
        SharedMemoryStatusNotOpened,   //failed to open shared memory 
        SharedMemoryStatusMismatch,    //version or platform incompatible, e.g., 2.7 VS 2.8, 32-bit VS 64-bit
        SharedMemoryStatusReopened     //reopened just now
    };

    static const uint32_t c_lockTimeoutInMS = 10000; 
}}}