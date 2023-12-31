# slamware_ros_bridge
### ros_bridge

An adapter between third-party ros node and slamware, run as ros node.

### slamware_sdk

header files and library files of slamware 



### How to build

1. decompress the library files according to your OS and architecture.

```bash
cd src/slamawre_sdk
tar -zxf x86_64_gcc5.4.tar.gz   //for Ubuntu 16.04
```



2. build ros node

```bash
cd ../..
catkin_make
```



3. run ros node

```bash
source devel/setup.bash
roslaunch slamware_ros_bridge slamware_ros_bridge.launch
```



### Topics

| Topic名称 | 作用         | 发布者     | 数据类型             |
| --------- | ------------ | ---------- | -------------------- |
| odom      | 底盘运动里程 | 第三方系统 | nav_msgs::Odometry   |
| cmd_vel   | 速度控制命令 | slamwared  | geometry_msgs::Twist |
