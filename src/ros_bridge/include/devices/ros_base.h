#pragma once

#include <rp/slamware/utils/pseudo_base_device.h>
#include <rpos/system/util/log.h>
#include <boost/shared_ptr.hpp>                 
#include <boost/weak_ptr.hpp>

namespace rp { namespace slamware { namespace utils {

    class DevicesManagerService;

    class RosBaseDevice
        : public PseudoBaseDevice
    {
    public:
        static const unsigned char DefaultBinaryConfig[];
        static const size_t DefaultBinaryConfigSize;

    public:
        explicit RosBaseDevice(boost::shared_ptr<DevicesManagerService> deviceManager, const std::string& binaryConfigFile = "");
        ~RosBaseDevice();

    public:
        virtual bool getBinaryConfig(std::uint8_t* buf, size_t* size);
        virtual bool requestMotion(const rpos::message::base::MotionRequest& request);
        virtual bool getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation);
        virtual bool getExtendedSensorData(std::vector<BaseSensorData>& extendedSensorData);
        virtual bool getBaseStatus(BaseStatusData& data);

    private:
        void initBinaryConfig_(const std::string& binaryConfigFile);
        void initDefaultBinaryConfig_();

    private:
        static rpos::system::util::LogScope logger;
        boost::weak_ptr<DevicesManagerService> deviceManager_;
        std::vector<unsigned char> binaryConfig_;
    };

}}}