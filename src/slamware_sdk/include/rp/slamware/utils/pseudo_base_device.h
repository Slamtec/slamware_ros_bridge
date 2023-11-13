/*
* pseudo_cp0_device.h
* Pseudo CP0 device
*
* Created by Tony Huang (tony@slamtec.com) at 2018-10-22
* Copyright 2018 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "pseudo_device_config.h"
#include <rpos/message/impact_messages.h>
#include <rpos/message/base_messages.h>
#include <rpos/message/message.h>

namespace rp { namespace slamware { namespace utils {

    class PSEUDO_DEVICE_API PseudoBaseDevice
        : public IInterchipVirtualDevice
    {
    public:
        struct BaseSensorData
        {
            float rawDist;
            bool valid;
        };
        struct BaseStatusData
        {
            bool isDockLocked;
            bool isCharging;
            int battery_percentage;
        };

    public:
        virtual ~PseudoBaseDevice();

    protected:
        PseudoBaseDevice();

    protected:
        /**
        * Get binary config data
        * IMPLEMENT THIS
        *
        * @param buf      The buffer to write to
        * @param size     Original value of size is the size of given buffer, you should update it to binary config's actual size
        * @return Indicate if binary config is written to buf successfully
        */
        virtual bool getBinaryConfig(uint8_t*buf, size_t* size) = 0;

        /**
        * Send motion request to the base
        * IMPLEMENT THIS
        *
        * @param request The motion request (vx, vy, omega), all of the values are in metric units, which are (m/s, m/s, rad/s)
        */
        virtual bool requestMotion(const rpos::message::base::MotionRequest& request) = 0;

        /**
        * Get movement estimation since last get
        * IMPLEMENT THIS
        *
        * @param estimation The data structure you should fill (dx, dy, dtheta), all of the values are in metric units, which are (m, m, rad
        */
        virtual bool getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation) = 0;

        /**
         * Get base sensor data
         * IMPLEMENT THIS
         *
         * @param estimation the data structure you should fill (rawDIst, valid), which the values are in metric units (m), the index of the vector will be the sensor id
         */
        virtual bool getExtendedSensorData(std::vector<BaseSensorData>& extSensorData) = 0;


        virtual bool getBaseStatus(BaseStatusData& data);

    public:
        virtual void handleRequest(std::uint8_t command, const void* data, size_t nbytes, IRequestContext& context);

        bool isClientConnected();

    private:
        uint64_t lastCmdTimeStamp_;
    };

} } }
