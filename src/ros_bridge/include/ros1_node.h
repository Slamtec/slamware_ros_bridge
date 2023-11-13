#pragma once

#include "config.h"
#include <rp/slamware/utils/pseudo_rplidar_device.h>
#include <rp/slamware/utils/pseudo_base_device.h>
#include <rpos/message/lidar_messages.h>
#include <rpos/message/base_messages.h>
#include <rpos/system/shared_memory/shared_memory.h>
#include <rpos/system/shared_memory/shm_topic_manager.h>
#include <rpos/core/pose.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <tf2/LinearMath/Transform.h>
#include <mutex>

namespace rp { namespace slamware { namespace utils {

    class Ros1NodeBase
    {
    protected:
        Ros1NodeBase(int argc, char **argv, const std::string& nodeName)
        {
            ros::init(argc, argv, nodeName);
        }
    };

    class Ros1Node : public Ros1NodeBase
    {
    public:
        Ros1Node(int argc, char** argv, const std::string& nodeName);
        ~Ros1Node();

    public:
        void initConfig(RosNodeConfig& cfg);
        void subscribe(std::string& msgTopic, std::uint32_t queueSize, MsgType msgType);
        void spin(bool isOnce);

        template <typename msgT>
        void advertise(std::string& msgTopic, std::uint32_t queueSize, MsgType msgType)
        {
            if (msgType == MsgTypeVelocity)
                pubVelocity_ = nh_.advertise<msgT>(msgTopic, queueSize);
        }

        template <typename msgT>
        void publish(const msgT& msg, MsgType msgType)
        {
            if (msgType == MsgTypeVelocity)
                pubVelocity_.publish(msg);
        }

	void clear();

    public:
        bool getLaserScan(rpos::message::lidar::LidarScan& lidarData);
        rpos::core::Vector3f getDeadReckon(uint64_t& timestamp);
        void registerLidarDevice(boost::shared_ptr<PseudoRPLidarDevice> lidarDevice){ lidarDevice_ = lidarDevice; }
        void registerBaseDevice(boost::shared_ptr<PseudoBaseDevice> baseDevice){ baseDevice_ = baseDevice; }

    private:
        void laserScanCallback_(const sensor_msgs::LaserScan::ConstPtr& msg);
        void odometryCallback_(const nav_msgs::Odometry::ConstPtr& msg);
        void deadReckonCallback_(const geometry_msgs::Vector3Stamped::ConstPtr& msg);
        void generateTimeOffset_();
        bool ensureSharedMemoryTopic_();
    private:
        ros::NodeHandle nh_;
        ros::Subscriber subLaserScan_;
        ros::Subscriber subOdometry_;
        ros::Publisher pubVelocity_;

        ros::Time startupSystemTime_;
        uint64_t startupSteadyTime_;

        std::mutex laserDataLock_;
        rpos::message::lidar::LidarScan laserScan_;

        bool isOdometry_;
        std::mutex poseDataLock_;
        uint64_t odomTimestamp_;
        bool initEstimationFlag_;
        tf2::Transform pose_;
        tf2::Transform prePose_;
        std::vector<rpos::core::Vector3f> deadreckon_;

        bool enable_shared_memory_;
        boost::shared_ptr<PseudoRPLidarDevice> lidarDevice_;
        boost::shared_ptr<PseudoBaseDevice> baseDevice_;
        boost::shared_ptr<rpos::system::shared_memory::ShmTopic<rpos::system::shared_memory::LaserScan> > topicLaserScan_;
        boost::shared_ptr<rpos::message::Message<rpos::system::shared_memory::LaserScan>> laserScanMsg_;
    };

}}}
