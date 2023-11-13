#include "ros1_node.h"
#include <rpos/core/angle_math.h>
#include <rpos/system/util/time_util.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <cmath>

namespace rp { namespace slamware { namespace utils {

    using namespace rpos::system::shared_memory;

    Ros1Node::Ros1Node(int argc, char** argv, const std::string& nodeName) 
        : Ros1NodeBase(argc, argv, nodeName)
        , initEstimationFlag_(true)
        , isOdometry_(true)
        , enable_shared_memory_(false)
    {
        generateTimeOffset_();
    }

    Ros1Node::~Ros1Node()
    {
        clear();
    }
 
    void Ros1Node::clear()
	{
		subLaserScan_ = ros::Subscriber();
        subOdometry_ = ros::Subscriber();
	}
    
    void Ros1Node::initConfig(RosNodeConfig& cfg)
    {
        cfg.setBy(nh_);
        enable_shared_memory_ = cfg.enable_shared_memory_lidar;
    }
    
    void Ros1Node::subscribe(std::string& msgTopic, std::uint32_t queueSize, MsgType msgType)
    {
        if (msgType == MsgTypeScan)
        {
            subLaserScan_ = nh_.subscribe(msgTopic, queueSize, &Ros1Node::laserScanCallback_, this);
        }
        else if (msgType == MsgTypeOdometry)
        {  
            subOdometry_ = nh_.subscribe(msgTopic, queueSize, &Ros1Node::odometryCallback_, this);
            isOdometry_ = true;
            ROS_INFO("subscribe odometry topic: %s", msgTopic.c_str());
        }
        else if ( msgType == MsgTypeDeadreckon)
        {
            subOdometry_ = nh_.subscribe(msgTopic, queueSize, &Ros1Node::deadReckonCallback_, this);
            isOdometry_ = false;
            ROS_INFO("subscribe deadreckon topic: %s", msgTopic.c_str());
        }
    }

    void Ros1Node::spin(bool isOnce)
    {
        if (isOnce)
            ros::spinOnce();
        else
            ros::spin();
    }

    bool Ros1Node::getLaserScan(rpos::message::lidar::LidarScan& lidarData)
    {
        ROS_ERROR("ros rplidar is callback mode, this function should never be called");
        return false;
    }

    rpos::core::Vector3f Ros1Node::getDeadReckon(uint64_t& timestamp)
    {
        double dx = 0.0;
        double dy = 0.0;
        double dyaw = 0.0;
        if(isOdometry_)
        {
            if (initEstimationFlag_)
            {
                std::lock_guard<std::mutex> lkGuard(poseDataLock_);
                initEstimationFlag_ = false;
                prePose_ = pose_;
                timestamp = odomTimestamp_;
            }
            else
            {
                std::lock_guard<std::mutex> lkGuard(poseDataLock_);
                tf2::Transform trans = prePose_.inverseTimes(pose_);
                dx = trans.getOrigin().getX();
                dy = trans.getOrigin().getY();
                double roll, pitch;
                trans.getBasis().getRPY(roll, pitch, dyaw);
                timestamp = odomTimestamp_;
                prePose_ = pose_;
            }
        }
        else
        {
            std::vector<rpos::core::Vector3f> deadreckons;
            {
                std::lock_guard<std::mutex> lkGuard(poseDataLock_);
                deadreckons.swap(deadreckon_);
                timestamp = odomTimestamp_;
            }
            for(auto odom : deadreckons)
            { 
                double costheta = cos(dyaw);
                double sinTheta = sin(dyaw);
                dx += odom.x() * costheta - odom.y()* sinTheta;
                dy += odom.x() * sinTheta + odom.y()* costheta;
                dyaw += odom.z();
            }
        } 
        dyaw = rpos::core::constraitRadNegativePiToPi(dyaw);

        return rpos::core::Vector3f(dx,dy,dyaw);
    }

    void Ros1Node::laserScanCallback_(const sensor_msgs::LaserScan::ConstPtr& msg)
    {
        int count = msg->scan_time / msg->time_increment; 

        count = std::min<int>(count, msg->ranges.size());
        
        auto duration = msg->header.stamp - startupSystemTime_; 
        int64_t ts = startupSteadyTime_ + duration.sec*1000 + duration.nsec/1000000;

        rpos::message::lidar::LidarScan laserScan; 

        bool sharedMemoryEnabled = ensureSharedMemoryTopic_();
        if(sharedMemoryEnabled)
        {
            laserScanMsg_->payload.data.clear();
            laserScanMsg_->timestamp = ts;
        }
        for (int i = 0; i < count; i++)
        {
            if (msg->ranges[i] < msg->range_min || msg->ranges[i] > msg->range_max)
                continue;

            float degree = msg->angle_min + msg->angle_increment * i;
            float rad = rpos::core::constraitRadZeroTo2Pi(degree + M_PI); //RPlidar ROS SDK inverse all the data

            rpos::message::lidar::LidarScanPoint lidarPoint;
            lidarPoint.dist = msg->ranges[i];
            lidarPoint.angle = rpos::core::rad2deg(rad);
            lidarPoint.valid = true;
	        lidarPoint.layer = "";
            laserScan.push_back(lidarPoint);

            if(sharedMemoryEnabled)
            {
                rpos::system::shared_memory::LaserPoint p;
                p.angle = rad;
                p.dist = lidarPoint.dist;
                p.valid = lidarPoint.valid;
                laserScanMsg_->payload.data.push_back(p);
            }
        }
        lidarDevice_->onScanDataReceived(std::move(laserScan)); 
        if(topicLaserScan_)
            topicLaserScan_->publish(*laserScanMsg_);
    }

    void Ros1Node::odometryCallback_(const nav_msgs::Odometry::ConstPtr& msg)
    {  
        auto duration = msg->header.stamp - startupSystemTime_;
        tf2::Transform pose;
        pose.setOrigin(tf2::Vector3(msg->pose.pose.position.x, msg->pose.pose.position.y, msg->pose.pose.position.z)); 
        tf2::Quaternion quat_tf;
        tf2::fromMsg(msg->pose.pose.orientation, quat_tf);
        pose.setRotation(quat_tf);
        std::lock_guard<std::mutex> lkGuard(poseDataLock_);
        pose_ = pose;
        odomTimestamp_ = startupSteadyTime_ + duration.sec*1000 + duration.nsec/1000000;
    }
    
    void Ros1Node::deadReckonCallback_(const geometry_msgs::Vector3Stamped::ConstPtr& msg)
    {
        auto duration = msg->header.stamp - startupSystemTime_;
        std::lock_guard<std::mutex> lkGuard(poseDataLock_);
        deadreckon_.emplace_back(msg->vector.x, msg->vector.y,msg->vector.z);
        odomTimestamp_ = startupSteadyTime_ + duration.sec*1000 + duration.nsec/1000000;
    }

    void Ros1Node::generateTimeOffset_()
    {
        startupSystemTime_ = ros::Time::now();
        startupSteadyTime_ = rpos::system::util::high_resolution_clock::get_time_in_ms();
    }

    bool Ros1Node::ensureSharedMemoryTopic_()
    {
        if(!enable_shared_memory_)
        {
            return false;
        }
        if(!baseDevice_->isClientConnected())
        {
            return false;
        }

        if (!SharedMemory::getInstance()->makeItValid(NULL))
        {
            return false;
        }
        if(topicLaserScan_== nullptr)
        {
            topicLaserScan_ = SharedMemory::getInstance()->getTopicManager()->getOrCreateTopic<LaserScan>("sensors/laser_scan", ShmTopicQos::ShmTopicQosMessageSingleton); 
            laserScanMsg_ = boost::make_shared<rpos::message::Message<rpos::system::shared_memory::LaserScan>>();
        }       
        return true;
    }

}}}

