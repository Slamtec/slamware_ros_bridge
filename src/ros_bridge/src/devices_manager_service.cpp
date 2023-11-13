#include "devices_manager_service.h"
#include "devices/ros_base.h"
#include "devices/ros_rplidar.h"
#include <boost/make_shared.hpp>
#include <chrono>

namespace rp { namespace slamware { namespace utils {

    const int DevicesManagerService::threadDuration = 1000 / 100;

    DevicesManagerService::DevicesManagerService()
        : rosNode_("rosNode"), working_(false)
        , bridge_(nullptr), lidar_(nullptr)
    {
        depends(&rosNode_);
    }

    DevicesManagerService::~DevicesManagerService()
    {
    }

    bool DevicesManagerService::onStart()
    {
        thread_ = std::thread(std::bind(&DevicesManagerService::workThread_, this));
        return true;
    }

    bool DevicesManagerService::onStop()
    {
	    logger.info_out("devices start stop.");
        if (working_.load())
            working_.store(false);

        if (thread_.joinable())
            thread_.join();

	    logger.info_out("devices end stop.");
        return true;
    }

    bool DevicesManagerService::getScanData(rpos::message::lidar::LidarScan& lidarData)
    {
        return rosNode_->getLaserScan(lidarData);
    }

    void DevicesManagerService::publishMotion(const rpos::message::base::MotionRequest& request)
    {
        rosNode_->publishMotion(request);
    }

    void DevicesManagerService::getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation)
    {
        rosNode_->getMovementEstimation(estimation);
    }

    void DevicesManagerService::workThread_()
    {
        logger.info_out("device manager service thread begin.");
        working_.store(true);
        if (!initBridge_())
        {
            cleanup_();
            return;
        }

        configDevices_();

        auto res = bridge_->start();
        if (IS_FAIL(res))
        {
            logger.error_out("can not start the server: %08x.", res);
            cleanup_();
            return;
        }

        while (working_.load())
        {
            bridge_->heartBeat();
            std::this_thread::sleep_for(std::chrono::milliseconds(threadDuration));
        }

        logger.info_out("devices manager service stop bridge.");
        bridge_->stop(); 
        cleanup_();
    }

    bool DevicesManagerService::initBridge_()
    {
        bridge_ = createBridgeServer();
        if (!bridge_)
            return false;

        u_result res = bridge_->init("slamware_pseudo_device_bridge", "Slamtec Slamware Pseudo Device Bridge");
        if (IS_FAIL(res))
        {
            logger.error_out("can not initialize bridge: %08x.", res);
            return false;
        }

        return true;
    }

    void DevicesManagerService::configDevices_()
    {
        lidar_ = boost::make_shared<RosRPLidarDevice>(shared_from_this());
        base_ = boost::make_shared<RosBaseDevice>(shared_from_this());
        bridge_->addVirtualDevice("rplidar", "Lidar", lidar_.get());
        bridge_->addVirtualDevice("ctrlbus", "Control Bus", base_.get());
        rosNode_->registerLidarDevice(lidar_);
        rosNode_->registerBaseDevice(base_);
    }

    void DevicesManagerService::cleanup_()
    {
        working_.store(false);

        if (bridge_)
        {
            releaseBridgeServer(bridge_);
            bridge_ = nullptr;
        }
    }
}}}
