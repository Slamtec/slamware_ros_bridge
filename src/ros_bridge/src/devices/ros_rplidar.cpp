#include "devices/ros_rplidar.h"
#include "devices_manager_service.h"

namespace rp { namespace slamware { namespace utils {

    RosRPLidarDevice::RosRPLidarDevice(boost::shared_ptr<DevicesManagerService> deviceManager)
        : deviceManager_(deviceManager)
    {
        //
    }

    RosRPLidarDevice::~RosRPLidarDevice()
    {
        //
    }

    bool RosRPLidarDevice::getScanData(rpos::message::lidar::LidarScan& lidarData)
    {
        if (deviceManager_.expired())
            return false;
        return deviceManager_.lock()->getScanData(lidarData);
    }

}}}
