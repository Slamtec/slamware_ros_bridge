#include "devices/ros_base.h"
#include "devices_manager_service.h"

#include <fstream>
#include <sstream>

namespace rp { namespace slamware { namespace utils {

    rpos::system::util::LogScope RosBaseDevice::logger("srv.ros_base_device");
    const unsigned char RosBaseDevice::DefaultBinaryConfig[] = {0x50,0x00};
    const size_t RosBaseDevice::DefaultBinaryConfigSize = 2;

    RosBaseDevice::RosBaseDevice(boost::shared_ptr<DevicesManagerService> deviceManager, const std::string& binaryConfigFile)
        : deviceManager_(deviceManager)
    {
        initBinaryConfig_(binaryConfigFile);
    }

    RosBaseDevice::~RosBaseDevice()
    {
        //
    }

    bool RosBaseDevice::getBinaryConfig(std::uint8_t* buf, size_t* size)
    {
        if (!buf || !size || (*size) < binaryConfig_.size())
            return false;

        (*size) = binaryConfig_.size();
        memcpy(buf, &binaryConfig_[0], binaryConfig_.size());

        return true;
    }

    bool RosBaseDevice::requestMotion(const rpos::message::base::MotionRequest& request)
    {
        if (deviceManager_.expired())
            return false;
        deviceManager_.lock()->publishMotion(request);
        return true;
    }

    bool RosBaseDevice::getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation)
    {
        if (deviceManager_.expired())
            return false;
        deviceManager_.lock()->getMovementEstimation(estimation);
        return true;
    }

    bool RosBaseDevice::getExtendedSensorData(std::vector<BaseSensorData>& extendedSensorData)
    {
        return true;
    }
    
    bool RosBaseDevice::getBaseStatus(BaseStatusData& data)
    {
        return true;
    }

    void RosBaseDevice::initBinaryConfig_(const std::string& binaryConfigFile)
    {
        if (binaryConfigFile.empty())
        {
            logger.info_out("binary config file not specified, load default binary config");
            initDefaultBinaryConfig_();
            return;
        }

        std::ifstream file(binaryConfigFile);
        if (file.fail())
        {
            logger.warn_out("fail to open file (%s), load default binary config", binaryConfigFile.c_str());
            initDefaultBinaryConfig_();
            return;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        auto  configContent = ss.str();
        binaryConfig_ = std::vector<unsigned char>(configContent.begin(), configContent.end());
        file.close();

        logger.info_out("success to load binary config file (%s), size %d", binaryConfigFile.c_str(), binaryConfig_.size());
    }

    void RosBaseDevice::initDefaultBinaryConfig_()
    {
        binaryConfig_ = std::vector<unsigned char>(DefaultBinaryConfig, DefaultBinaryConfig + DefaultBinaryConfigSize);
    }

}}}