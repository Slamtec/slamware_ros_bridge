#include "ros_node_service.h"
#if ROS_DISTRO_VERSION == 1 
#include "ros1_node.h"
#endif  
#include <rpos/system/util/log.h>
#include <geometry_msgs/Twist.h>
#include <thread>
#include <chrono>
#include <cmath>

using namespace rpos::context;

namespace {
    const float kPif = 3.141592653;
}

namespace rp { namespace slamware { namespace utils {
 
    template <typename RosHandlerT>
    RosNodeService<RosHandlerT>::RosNodeService(int argc, char** argv)
        : initEstimationFlag_(true)
    {
        this->template provides<IRosNode>();
        rosNode_.reset(new RosHandlerT(argc, argv, "slamware_ros_bridge_node"));
    }

    template <typename RosHandlerT>
    RosNodeService<RosHandlerT>::~RosNodeService()
    {
        
    }

    template <typename RosHandlerT>
    bool RosNodeService<RosHandlerT>::onStart()
    {
        if (!rosNode_ )
            return false;
        rosNode_->initConfig(config_);

        logger.info_out("ros node service thread begin, lidar topic:%s, odom topic:%s, velocity command topic:%s", 
            config_.scan_sub_topic.c_str(),config_.odometry_sub_topic.c_str(), config_.velocity_pub_topic.c_str());

        rosNode_->subscribe(config_.scan_msg_topic, 1, MsgType::MsgTypeScan);
        if(config_.is_accumulated_odometry){
            rosNode_->subscribe(config_.odometry_sub_topic, 10, MsgType::MsgTypeOdometry);
        }
        else{
            rosNode_->subscribe(config_.odometry_sub_topic, 10, MsgType::MsgTypeDeadreckon);
        }
        rosNode_->template advertise<geometry_msgs::Twist>(config_.velocity_pub_topic, 10, MsgType::MsgTypeVelocity);

        return true;
    }

    template <typename RosHandlerT>
    bool RosNodeService<RosHandlerT>::onStop()
    {
	    logger.info_out("ros service stop.");
	    rosNode_->clear();
        return true;
    }

    template <typename RosHandlerT>
    void RosNodeService<RosHandlerT>::spin()
    {
	    rosNode_->spin(false);
    }

    template <typename RosHandlerT>
    bool RosNodeService<RosHandlerT>::getLaserScan(rpos::message::lidar::LidarScan& lidarData)
    {
        return rosNode_->getLaserScan(lidarData);
    }

    template <typename RosHandlerT>
    void RosNodeService<RosHandlerT>::publishMotion(const rpos::message::base::MotionRequest& request)
    {
        geometry_msgs::Twist msg;
        msg.linear.x = request.vx();
        msg.linear.y = request.vy();
        msg.linear.z = 0.0f;
        msg.angular.x = 0.0f;
        msg.angular.y = 0.0f;
        msg.angular.z = request.omega();
        rosNode_->template publish<geometry_msgs::Twist>(msg, MsgType::MsgTypeOdometry);
    }

    template <typename RosHandlerT>
    void RosNodeService<RosHandlerT>::getMovementEstimation(rpos::message::Message<rpos::message::base::MovementEstimation>& estimation)
    { 
        uint64_t timestamp;
        auto odom = rosNode_->getDeadReckon(timestamp);
        estimation->positionDifference.x() = odom.x();
        estimation->positionDifference.y() = odom.y();
        estimation->angularDifference = odom.z();
        estimation.timestamp = timestamp;
    }

    template <typename RosHandlerT>
    void RosNodeService<RosHandlerT>::registerLidarDevice(boost::shared_ptr<PseudoRPLidarDevice> device)
    {
        rosNode_->registerLidarDevice(device);
    }

    template <typename RosHandlerT>
    void RosNodeService<RosHandlerT>::registerBaseDevice(boost::shared_ptr<PseudoBaseDevice> device)
    {
        rosNode_->registerBaseDevice(device);
    }

}}}

template class rp::slamware::utils::RosNodeService<rp::slamware::utils::Ros1Node>;


