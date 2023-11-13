#include "devices_manager_service.h"
#include "ros_node_service.h"
#if ROS_DISTRO_VERSION == 1 
#include "ros1_node.h"
#endif  
#include <rpos/system/util/log.h>
#include <rpos/system/config/options_parser.h>
#include <rpos/system/shared_memory/shm_topic_manager.h>
#include <rpos/context/application_context.h>
#include <boost/make_shared.hpp>
#include <chrono>
#include <thread>
#include <csignal>

rpos::context::ApplicationContext context;

int main(int argc, char** argv)
{
    auto logManager = rpos::system::util::LogManager::defaultManager();
    logManager->addConsoleAppender(); 

    boost::shared_ptr<rp::slamware::utils::IRosNode> rosNodeService;
#if ROS_DISTRO_VERSION == 1 
    rosNodeService = boost::make_shared<rp::slamware::utils::RosNodeService<rp::slamware::utils::Ros1Node>>(argc, argv);
    context.addService(boost::dynamic_pointer_cast<rp::slamware::utils::RosNodeService<rp::slamware::utils::Ros1Node>>(rosNodeService));
#elif ROS_DISTRO_VERSION == 2
    #pragma message("ROS 2 not supported")
#else
    #pragma message("ROS version not defined")
#endif

    if(rosNodeService && rosNodeService->config().enable_shared_memory_lidar)
    {
        rpos::system::shared_memory::SharedMemory::getInstance()->init(rpos::system::shared_memory::MemoryAccessRightUser);
    }
    auto deviceManagerService = boost::make_shared<rp::slamware::utils::DevicesManagerService>();
        
    context.addService(deviceManagerService);
    context.startServices();

    if(rosNodeService)
    {
        rosNodeService->spin();
    }
 
    context.stopServices(); 
    return 0;
}