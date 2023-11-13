#pragma once

#include <rp/slamware/utils/pseudo_rplidar_device.h>
#include <boost/shared_ptr.hpp>                 
#include <boost/weak_ptr.hpp>

namespace rp { namespace slamware { namespace utils {

    class DevicesManagerService;

    class RosRPLidarDevice
        : public PseudoRPLidarDevice
    {
    public:
        explicit RosRPLidarDevice(boost::shared_ptr<DevicesManagerService> deviceManager);
        ~RosRPLidarDevice();

    protected:
        virtual bool getScanData(rpos::message::lidar::LidarScan& lidarData);

    private:
        boost::weak_ptr<DevicesManagerService> deviceManager_;
    };
}}}