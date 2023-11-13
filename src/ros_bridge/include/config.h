#pragma once
#include <string>

#define ROS_DISTRO_VERSION 1

#if ROS_DISTRO_VERSION == 1
#include <ros/ros.h>
#endif

namespace rp { namespace slamware { namespace utils {

    enum MsgType
    {
        MsgTypeScan,
        MsgTypeOdometry,
        MsgTypeDeadreckon,
        MsgTypeVelocity
    };

    struct RosNodeConfig
    {
        std::string scan_sub_topic;
        std::string odometry_sub_topic;
        std::string velocity_pub_topic;
        bool is_accumulated_odometry;
        bool enable_shared_memory_lidar;
        
        RosNodeConfig();
        void resetToDefault();
#if ROS_DISTRO_VERSION == 1
        void setBy(const ros::NodeHandle& nhRos);        
#endif
    };

}}}
