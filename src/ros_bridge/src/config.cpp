#include "config.h"

namespace rp { namespace slamware { namespace utils {
 
    RosNodeConfig::RosNodeConfig()
    {
        resetToDefault();
    }

    void RosNodeConfig::resetToDefault()
    {
        scan_sub_topic = "scan";
        odometry_sub_topic = "odom";
        is_accumulated_odometry = true;
        velocity_pub_topic = "cmd_vel";
        enable_shared_memory_lidar = false;
    }

#if ROS_DISTRO_VERSION == 1
    void RosNodeConfig::setBy(const ros::NodeHandle& nhRos)
    {
        nhRos.getParam("scan_sub_topic", scan_sub_topic);
        nhRos.getParam("odometry_sub_topic", odometry_sub_topic);
        nhRos.getParam("is_accumulated_odometry", is_accumulated_odometry);
        nhRos.getParam("velocity_pub_topic", velocity_pub_topic);
        nhRos.getParam("enable_shared_memory_lidar", enable_shared_memory_lidar);
    }
#endif
}}}
