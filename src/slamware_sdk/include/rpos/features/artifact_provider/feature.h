/*
* feature.h
* Artifact Provider feature
*
* Created by Jacky Li (eveningwear@gmail.com) at 2014-12-26
* Copyright 2014 (c) www.robopeak.com
*/

#pragma once

#include <rpos/rpos_config.h>

#include <rpos/core/feature.h>
#include <rpos/core/geometry.h>
#include <rpos/core/pose.h>
#include <rpos/core/metadata.h>
#include <rpos/core/pose_entry.h>

#include <vector>
#include "rectangle_area.h"

namespace rpos { namespace features {

    namespace detail {
        class ArtifactProviderImpl;
    }

    namespace artifact_provider
    {
        enum ArtifactUsage
        {
            /**
            * Virtual walls works just like walls, they blocks robot move through
            */
            ArtifactUsageVirtualWall,

            /**
            * Virtual tracks are used to guide robot's movement. Robot will try its best to move along with virtual tracks
            */
            ArtifactUsageVirtualTrack,

            /**
            * Schedule areas are used to achieve multi-robot collaborative behavior
            */
            ArtifactUsageScheduleArea,
            
            /**
            * Maintenance area has mutable environment, slamware will automatically update map of this area
            */
            ArtifactUsageMaintenanceArea,
            
            /**
            * Forbidden area means robot should not enter this area, but if accidentally entered, it can escape
            */
            ArtifactUsageForbiddenArea,

            /**
            * Robot will disable designated sensor when entering sensor disable area
            */
            ArtifactUsageSensorDisableArea,
            
            /**
            * Elevator area is used to describe the area associated with the elevator, mainly including the sill border, the inner border and outer border.
            */
            ArtifactUsageElevatorArea,

            /**
            * When in dangerous area some limits will be enabled, such as max_speed, max_acceleration.
            */
            ArtifactUsageDangerousArea,

            /**
            * points of interest
            */
            ArtifactUsagePoi,

            /**
            * Coverage area for sweeping and disinfecting
            */
            ArtifactUsageCoverageArea,

            /**
            * Restricted area for multi-robot collaborative behavior
            */
            ArtifactUsageRestrictedArea,

            /**
            * special marks detected by lidar or camera
            */
            ArtifactUsageLandmark
        };

        struct RPOS_CORE_API RectangleArea {
            rpos::core::SegmentID id;
            rpos::core::ORectangleF area;
            ArtifactUsage usage;
            rpos::core::Metadata metadata;
            RectangleArea();
            RectangleArea(ArtifactUsage usage, const rpos::core::Point& startP, const rpos::core::Point& endP, float halfWidth);
            RectangleArea(ArtifactUsage usage, const rpos::core::ORectangleF& rect, rpos::core::SegmentID id);
        };
    }

    class RPOS_CORE_API ArtifactProvider : public rpos::core::Feature{
    public:
        typedef detail::ArtifactProviderImpl impl_t;

        RPOS_OBJECT_CTORS_WITH_BASE(ArtifactProvider, rpos::core::Feature);
        ArtifactProvider(boost::shared_ptr<detail::ArtifactProviderImpl> impl);
        ~ArtifactProvider();

    public:
        std::vector <artifact_provider::RectangleArea> getRectangleAreas(artifact_provider::ArtifactUsage usage);
        bool addRectangleArea(artifact_provider::ArtifactUsage usage, const artifact_provider::RectangleArea& area);
        bool editRectangleArea(artifact_provider::ArtifactUsage usage, const artifact_provider::RectangleArea& area);
        bool addRectangleAreas(artifact_provider::ArtifactUsage usage, const std::vector<artifact_provider::RectangleArea>& areas);
        bool removeRectangleAreaByIds(artifact_provider::ArtifactUsage usage, const std::vector<core::SegmentID>& ids);
        bool clearRectangleAreas(artifact_provider::ArtifactUsage usage);

        std::vector<core::Line> getLines(artifact_provider::ArtifactUsage usage);
        bool addLine(artifact_provider::ArtifactUsage usage, const core::Line& line);
        bool addLines(artifact_provider::ArtifactUsage usage, const std::vector<core::Line>& lines);
        bool removeLineById(artifact_provider::ArtifactUsage usage, rpos::core::SegmentID id);
        bool clearLines(artifact_provider::ArtifactUsage usage);
        bool moveLine(artifact_provider::ArtifactUsage usage, const core::Line& line);
        bool moveLines(artifact_provider::ArtifactUsage usage, const std::vector<core::Line>& lines);

        std::vector<core::Line> getWalls();
        bool addWall(const core::Line& wall);
        bool addWalls(const std::vector<core::Line>& wall);
        bool clearWallById(const rpos::core::SegmentID& id);
        bool clearWalls();

        std::vector<core::RectangleF> getMapDiscrepancyMonitorAreas();
        bool addMapDiscrepancyMonitorArea(const core::RectangleF& area);
        bool addMapDiscrepancyMonitorAreas(const std::vector<core::RectangleF>& areas);
        bool removeMapDiscrepancyMonitorAreas(const std::vector<core::RectangleF>& areas);
        bool clearMapDiscrepancyMonitorAreas();

        bool setPOIs(const core::PoseEntryMap& pois);
        core::PoseEntryMap getPOIs();
        bool addPOI(const core::PoseEntry& poi);
        bool editPOI(const core::PoseEntry& poi);
        core::PoseEntry addPOIOnCurrentPose(const std::string& name, const core::Metadata& metadata);
        std::pair<std::string, core::PoseEntry> queryPOI(const std::string& name);
        std::pair<std::string, core::PoseEntry> queryClosestPOI(const rpos::core::Pose& pose);
        bool erasePOI(const std::string& name);
        bool clearPOIs();
        bool adjustPOIs();

        std::vector<rpos::core::PoseEntry> getLaserLandmarks();
        bool setLaserLandmarks(const std::vector<rpos::core::PoseEntry>& landmarks);
        std::vector<rpos::core::PoseEntry> getCurrentLaserLandmarks();
        bool setLaserLandmarkUpdate(bool update);
        bool getLaserLandmarkUpdate();
        bool removeLaserLandmarks(const std::vector<core::SegmentID>& ids);
        bool clearLaserLandmarks();
    };
} }
