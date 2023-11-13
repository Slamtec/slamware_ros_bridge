cmake_minimum_required(VERSION 2.8.3)
project(slamware_ros_bridge)

find_path(slamware_sdk_INCLUDE_DIR rpos/rpos.h ${PROJECT_SOURCE_DIR}/../slamware_sdk/include)
find_path(slamware_sdk_LIBRARY librpos_framework.a ${PROJECT_SOURCE_DIR}/../slamware_sdk/${CMAKE_HOST_SYSTEM_PROCESSOR}_lib)
if(slamware_sdk_INCLUDE_DIR AND slamware_sdk_LIBRARY)
  set(SLTC_SDK_INC_DIR "${slamware_sdk_INCLUDE_DIR}")
  set(SLTC_SDK_LIB_DIR "${slamware_sdk_LIBRARY}")
else()
  message(FATAL_ERROR "slamware sdk directory not found")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++11")

find_package(catkin REQUIRED COMPONENTS
  nav_msgs
  roscpp
  rospy
  sensor_msgs
  std_msgs
)

find_package(Threads)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES slamware_ros_bridge
  CATKIN_DEPENDS nav_msgs roscpp rospy sensor_msgs std_msgs tf message_runtime
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)
message("${INCLUDE_DIRECTORIES}")

add_executable(slamware_ros_bridge_node
  src/devices_manager_service.cpp
  src/devices/ros_base.cpp
  src/devices/ros_rplidar.cpp
  src/config.cpp
  src/ros1_node.cpp
  src/ros_node_service.cpp
  src/main.cpp
)
target_include_directories(slamware_ros_bridge_node
  PRIVATE ${SLTC_SDK_INC_DIR}
)
target_compile_options(slamware_ros_bridge_node
  PRIVATE -Wno-deprecated-declarations
)
target_link_libraries(slamware_ros_bridge_node
  ${SLTC_SDK_LIB_DIR}/libpseudo_device.a
  ${SLTC_SDK_LIB_DIR}/liblibany2tcp_bridge.a
  ${SLTC_SDK_LIB_DIR}/libslamware_sdp_driver.a
  ${SLTC_SDK_LIB_DIR}/libserial_tcp_client.a
  ${SLTC_SDK_LIB_DIR}/libinfra.a
  ${SLTC_SDK_LIB_DIR}/librpos_robotplatforms_rpslamware.a
  ${SLTC_SDK_LIB_DIR}/librpos_framework.a
  ${SLTC_SDK_LIB_DIR}/libbase64.a
  ${SLTC_SDK_LIB_DIR}/librlelib.a
  ${SLTC_SDK_LIB_DIR}/libjsoncpp.a
  ${SLTC_SDK_LIB_DIR}/libcurl.a
  ${SLTC_SDK_LIB_DIR}/libcares.a
  ${SLTC_SDK_LIB_DIR}/libssl.a
  ${SLTC_SDK_LIB_DIR}/libcrypto.a
  ${SLTC_SDK_LIB_DIR}/libboost_atomic.a
  ${SLTC_SDK_LIB_DIR}/libboost_chrono.a
  ${SLTC_SDK_LIB_DIR}/libboost_date_time.a
  ${SLTC_SDK_LIB_DIR}/libboost_regex.a 
  ${SLTC_SDK_LIB_DIR}/libboost_filesystem.a
  ${SLTC_SDK_LIB_DIR}/libboost_system.a
  ${SLTC_SDK_LIB_DIR}/libboost_thread.a
  ${SLTC_SDK_LIB_DIR}/libboost_random.a
  ${SLTC_SDK_LIB_DIR}/libusb-1.0.a
  ${SLTC_SDK_LIB_DIR}/libz.a
  ${catkin_LIBRARIES}
  pthread
  dl
  rt
)
