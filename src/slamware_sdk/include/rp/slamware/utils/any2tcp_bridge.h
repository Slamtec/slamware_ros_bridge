/*
* any2tcp_bridge.h
* Public APIs for Slamtec Any2Tcp Bridge
*
* Created by Tony Huang (tony@slamtec.com) at 2018-10-20
* Copyright 2018 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include <rpos/rpos_config.h>
#include <json/json.h>
#include <cstdint>
#include <rpos/message/lidar_messages.h>

#ifdef ANY2TCP_BRIDGE_DLL
#   ifdef ANY2TCP_BRIDGE_EXPORT
#       define ANY2TCP_BRIDGE_API RPOS_MODULE_EXPORT
#   else
#       define ANY2TCP_BRIDGE_API RPOS_MODULE_IMPORT
#   endif
#else
#	define ANY2TCP_BRIDGE_API
#endif

#ifndef u_result
typedef uint32_t u_result;

#define RESULT_OK                       0
#define RESULT_FAIL_BIT                 0x80000000
#define RESULT_ALREADY_DONE             0x20
#define RESULT_INVALID_DATA             (0x8000 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_FAIL           (0x8001 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_TIMEOUT        (0x8002 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_STOP           (0x8003 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_NOT_SUPPORT    (0x8004 | RESULT_FAIL_BIT)
#define RESULT_FORMAT_NOT_SUPPORT       (0x8005 | RESULT_FAIL_BIT)
#define RESULT_INSUFFICIENT_MEMORY      (0x8006 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_ABORTED        (0x8007 | RESULT_FAIL_BIT)
#define RESULT_NOT_FOUND                (0x8008 | RESULT_FAIL_BIT)
#define RESULT_RECONNECTING             (0x8009 | RESULT_FAIL_BIT)

#define IS_OK(x)    ( ((x) & RESULT_FAIL_BIT) == 0 )
#define IS_FAIL(x)  ( ((x) & RESULT_FAIL_BIT) )
#endif

namespace rp { namespace slamware { namespace utils {

    enum VirtualDeviceType {
        VirtualDeviceTypeUnknown,
        VirtualDeviceTypeInterchip,
        VirtualDeviceTypeRPLidar
    };

    class IVirtualDevice {
    public:
        explicit IVirtualDevice(VirtualDeviceType type);
        virtual ~IVirtualDevice();
        virtual VirtualDeviceType type();

    protected:
        VirtualDeviceType type_;
    };

    /**
    * A virtual device handles interchip protocols
    */
    class ANY2TCP_BRIDGE_API IInterchipVirtualDevice : public IVirtualDevice {
    public:
        IInterchipVirtualDevice();
        virtual ~IInterchipVirtualDevice();

    public:
        class ANY2TCP_BRIDGE_API IRequestContext {
        public:
            virtual ~IRequestContext();

        public:
            virtual void replySuccess(const void* data, size_t nbytes) = 0;
            virtual void replyError(std::uint16_t errorCode) = 0;
        };

        /**
        * The method you have to implement to handle requests
        */
        virtual void handleRequest(std::uint8_t command, const void* data, size_t nbytes, IRequestContext& context) = 0;
    };

    /**
    * A virtual device handles rplidar protocol
    */
    class ANY2TCP_BRIDGE_API IRPLidarVirtualDevice : public IVirtualDevice {
    public:
        IRPLidarVirtualDevice();
        virtual ~IRPLidarVirtualDevice();

    public:
        class ANY2TCP_BRIDGE_API IRequestContext {
        public:
            virtual ~IRequestContext();

        public:
            virtual void reply(std::uint8_t type, std::uint8_t flags, const void* data, size_t nbytes) = 0;
            virtual void replyError(std::uint16_t errorCode) = 0;
            virtual void initDataPacker(std::uint8_t scanModeAnswerType) = 0;
            virtual void sendScanData(const rpos::message::lidar::LidarScan& data, bool needSync) = 0;
        };

        /**
        * The method you have to implement to handle requests
        */
        virtual void handleRequest(std::uint8_t command, const void* data, size_t nbytes, IRequestContext& context) = 0;
    };

    /**
    * Abstract interface of the bridge server
    */
    class ANY2TCP_BRIDGE_API IBridgeServer {
    public:
        virtual ~IBridgeServer();

    public:
        /**
        * Init with config from config file
        */
        virtual u_result initWithConfigFile(const char* configFilename) = 0;

        /**
        * Init with server port
        */
        virtual u_result init(const char* serverName, const char* serverDescription, int serverPort = 9310) = 0;

        /**
        * Start the server
        */
        virtual u_result start() = 0;

        /**
        * Stop the server
        */
        virtual u_result stop() = 0;

        /**
        * Check if the server is working
        */
        virtual bool isWorking() = 0;

        /**
        * Heartbeat
        */
        virtual void heartBeat() = 0;

    public:
        /**
        * Clear all services
        * Notice: you may not modify service list once you connected to devices
        */
        virtual u_result clearServices() = 0;

        /**
        * Add physical device service
        * Notice: you may not modify service list once you connected to devices
        *
        * @param name       The name of the service (which will be used for service lookup)
        * @param desc       The description of the service (for human readability)
        * @param config     The config of the physical device
        * @param tcpNoDelay Set the NODELAY option to the socket
        */
        virtual u_result addPhysicalDevice(
            const std::string& name, 
            const std::string& desc, 
            const Json::Value& config, 
            bool tcpNoDelay = true
        ) = 0;

        /**
        * Add virtual device
        * Notice: The server will not handle the lifespan of the device instance, please release the device after stopping server and clearing services
        *
        * @param name       The name of the service (which will be used for service lookup)
        * @param desc       The description of the service (for human readability)
        * @param device     The virtual device
        * @param tcpNoDelay Set the NODELAY option to the socket
        */
        virtual u_result addVirtualDevice(
            const std::string& name,
            const std::string& desc,
            IVirtualDevice* device,
            bool tcpNoDelay = true
        ) = 0;
    };

    /**
    * Create a bridge server instance
    * Please use releaseBridgeServer() to release
    */
    ANY2TCP_BRIDGE_API IBridgeServer* createBridgeServer();

    /**
    * Release a bridge server instance
    * @param server The server instance to release
    */
    ANY2TCP_BRIDGE_API void releaseBridgeServer(IBridgeServer* server);

} } }
