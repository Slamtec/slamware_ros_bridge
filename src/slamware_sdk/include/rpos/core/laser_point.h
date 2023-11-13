/**
* laser_point.h
* Laser Point
*
* Created By Jacky Li @ 2014-7-20
* Copyright (c) 2014 Shanghai SlamTec Co., Ltd.
*/

#pragma once

#include <rpos/rpos_config.h>
#include <rpos/core/rpos_core_config.h>
#include <cstdint>

namespace rpos {
    namespace core {

        class RPOS_CORE_API LaserPoint{
        public:
            LaserPoint();
            LaserPoint(float distance, float angle, bool valid, std::uint8_t quality = 0);
            LaserPoint(const LaserPoint&);
            ~LaserPoint();

        public:
            LaserPoint& operator=(const LaserPoint&);

        public:
            float distance() const;
            float& distance();

            float angle() const;
            float& angle();

            bool valid() const;
            bool& valid();

            std::uint8_t quality() const;
            std::uint8_t& quality();

        private:
            float distance_;
            float angle_;
            bool valid_;
            std::uint8_t quality_;
        };
    }
}
