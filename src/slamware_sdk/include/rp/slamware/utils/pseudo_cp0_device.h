/*
* pseudo_cp0_device.h
* Pseudo CP0 device
*
* Created by Tony Huang (tony@slamtec.com) at 2018-10-22
* Copyright 2018 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "pseudo_device_config.h"
#include <rpos/message/imu_messages.h>

namespace rp { namespace slamware { namespace utils {

    class PSEUDO_DEVICE_API PseudoCp0Device
        : public IInterchipVirtualDevice
    {
    public:
        virtual ~PseudoCp0Device();

    protected:
        PseudoCp0Device();

    protected:
        /**
        * Get the IMU sensor data
        * IMPLEMENT THIS
        *
        * @param sensorData The data structure should be filled in this method
        *
        * Notice:
        * If your robot doesn't have an IMU, please fill all fields with zeros, but fill sensorData.gyroIntegral.z() to be acumulated value of deadreckon's dtheta
        */
        virtual void getImuAllSensorData(rpos::message::imu::ImuAllSensorData& sensorData) = 0;

    public:
        virtual void handleRequest(std::uint8_t command, const void* data, size_t nbytes, IRequestContext& context);
    };

} } }
