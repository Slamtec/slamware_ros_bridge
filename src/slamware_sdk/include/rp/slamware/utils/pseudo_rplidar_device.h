#pragma once
#include <rp/slamware/utils/pseudo_device_config.h>
#include <rpos/message/lidar_messages.h>
#include <boost/atomic.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace rp { namespace slamware { namespace utils {

    class PSEUDO_DEVICE_API PseudoRPLidarDevice
        : public IRPLidarVirtualDevice
    {
    public:
        enum WorkingMode
        {
            WorkingModeLegacy = 0,
            WorkingModeExpress = 1,
            WorkingModeBoost = 2,
            WorkingModeSensitivity = 3,
            WorkingModeStability = 4,
            WorkingModeCalib = 5,

            WorkingModeMax = WorkingModeCalib,
        };

        struct FirmwareInfo
        {
            uint8_t versionMajor;
            uint8_t versionMinor;
            uint8_t versionBuildNum;
            std::string surffixStr;
            std::string buildDate;
            std::string buildTime;
        };

        struct DeviceInfo
        {
            uint8_t hardwareVersion;
            uint8_t modelType;
            uint8_t subModelType;
            uint32_t hardwareBitmap;
            uint16_t peakThreshold;
            std::string serialNum;
        };

        struct ScanMode {
            uint32_t scanModeId;
            uint32_t usPerSample; // microseconds per sample  q8
            uint32_t maxDistance; // max distance  q8
            uint8_t answerType; // the answer type of the scam mode, its value should be RPLIDAR_ANS_TYPE_MEASUREMENT*
            char modeName[64]; // name of scan mode, max 63 characters
        };
        
        struct AngleRange
        {
            uint16_t minRange;  //q14
            uint16_t maxRange;  //q14
            uint16_t headingAngle;  //q14
        };

        struct DesiredRotFreq
        {
            uint16_t rpm;
            uint16_t pwmRef;
        };

        struct DeviceConfig
        {
            uint8_t principle;
            AngleRange angleRange;
            DesiredRotFreq desiredRotFreq;
            uint16_t minRotFreq;
            uint16_t maxRotFreq;
            uint32_t maxDistance;  //q8

            uint16_t typicalScanMode;
            std::vector<ScanMode> scanModes;
        };

    public:
        virtual ~PseudoRPLidarDevice();

    protected:
        PseudoRPLidarDevice();

    protected:
        /**
        * Get the lidar sensor data
        * IMPLEMENT THIS
        *
        * @param sensorData The data structure should be filled in this method
        *
        */
        virtual bool getScanData(rpos::message::lidar::LidarScan& lidarData) = 0;

    public:
        /*
        * if this function is invoked, it will switched to callback mode, 
        * getScanData is no longer needed 
        */
        void onScanDataReceived(rpos::message::lidar::LidarScan&& lidarData);

        virtual void handleRequest(std::uint8_t command, const void* data, size_t nbytes, IRequestContext& context);

    private:
        void init_();
        void initFirmwareInfo_();
        void initDeviceInfo_();
        void initConfig_();
        
        int checkWorkingModeSupported_(WorkingMode mode);
        uint8_t getModelType_();
        uint8_t getFirmwareMajor_();
        uint8_t getFirmwareMinor_();
        const char* getFirmwareSuffixStr_();
        const char* getFirmwareBuildDate_();
        const char* getFirmwareBuildTime_();
        uint8_t getHardwareVersion_();
        const char* getSerialNum_();
        std::vector<ScanMode>& getScanModes_();
        uint16_t getTypicalScanMode_();

        void workingThread_(WorkingMode mode, IRequestContext* context);

    private:
        FirmwareInfo firmwareInfo_;
        DeviceInfo deviceInfo_;
        DeviceConfig config_;

        boost::atomic<bool> working_;
        rpos::message::lidar::LidarScan scanData_;
        bool isCallbackMode_;
        std::mutex lock_;
        std::condition_variable signal_;
        std::thread scanThread_;
    };
    
}}}