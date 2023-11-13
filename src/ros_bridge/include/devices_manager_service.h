#pragma once

#include "ros_node_service.h"
#include <rpos/context/base_service.h>
#include <rpos/context/service_dependency.h>
#include <rpos/rpos.h>
#include <rp/slamware/utils/pseudo_device.h>
#include <rp/slamware/utils/pseudo_base_device.h>
#include <boost/shared_ptr.hpp>
#include <atomic>
#include <thread>

namespace rp { namespace slamware { namespace utils {

    class DevicesManagerService
        : public boost::enable_shared_from_this<DevicesManagerService>
        , public rpos::context::BaseService<DevicesManagerService>
    {
    public:
        explicit DevicesManagerService();
        virtual ~DevicesManagerService();

        virtual bool onStart();
        virtual bool onStop();

        bool getScanData(rpos::message::lidar::LidarScan& lidarData);
        void publishMotion(const rpos::message::base::MotionRequest& request);
        void getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation);

    private:
        void workThread_();
        bool initBridge_();
        void configDevices_();
        void cleanup_();

    private:
        static const int threadDuration;

        rpos::context::ServiceDependency<IRosNode> rosNode_;

        IBridgeServer* bridge_;
        std::atomic<bool> working_;
        std::thread thread_;

        boost::shared_ptr<PseudoRPLidarDevice> lidar_;
        boost::shared_ptr<PseudoBaseDevice> base_;
    };

}}}