#pragma once

#include "config.h"
#include <rp/slamware/utils/pseudo_rplidar_device.h>
#include <rp/slamware/utils/pseudo_base_device.h>
#include <rpos/message/lidar_messages.h>
#include <rpos/message/base_messages.h>
#include <rpos/core/pose.h>
#include <rpos/context/base_service.h>
#include <rpos/rpos.h>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

namespace rp { namespace slamware { namespace utils {

    class IRosNode
    {
    public:
        virtual ~IRosNode() {}
        virtual void spin() = 0;
        virtual bool getLaserScan(rpos::message::lidar::LidarScan& lidarData) = 0;
        virtual void publishMotion(const rpos::message::base::MotionRequest& request) = 0;
        virtual void getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation) = 0;
        virtual void registerLidarDevice(boost::shared_ptr<PseudoRPLidarDevice> lidarDevice) = 0;
        virtual void registerBaseDevice(boost::shared_ptr<PseudoBaseDevice> baeDevice) = 0;
        virtual const RosNodeConfig& config() = 0;
    };

    template <typename RosHandlerT>
    class RosNodeService
        : public IRosNode
        , public boost::enable_shared_from_this<RosNodeService<RosHandlerT>>
        , public rpos::context::BaseService<IRosNode>
    {
    public:
        explicit RosNodeService(int argc, char** argv);
        virtual ~RosNodeService();

        virtual bool onStart();
        virtual bool onStop();

        virtual void spin();
        virtual bool getLaserScan(rpos::message::lidar::LidarScan& lidarData);
        virtual void publishMotion(const rpos::message::base::MotionRequest& request);
        virtual void getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation);
        virtual void registerLidarDevice(boost::shared_ptr<PseudoRPLidarDevice> lidarDevice);
        virtual void registerBaseDevice(boost::shared_ptr<PseudoBaseDevice> baeDevice);
        virtual const RosNodeConfig& config() { return config_;}
 
    private: 
        RosNodeConfig config_;

        std::shared_ptr<RosHandlerT> rosNode_;
        bool initEstimationFlag_;
        float dXSinceLastFetch_;
        float dYSinceLastFetch_;
        float dYawSinceLastFetch_;
        std::mutex dOdomDataLock_;
    };

}}}

