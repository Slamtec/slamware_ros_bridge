/*
* base_message_serialization.h
* Base message diagnosis serialization
*
* Created by Tony Huang (tony@slamtec.com) at 2017-3-15
* Copyright 2017 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "../depth_camera_messages.h"
#include <rpos/system/diagnosis/diagnosis_serialization.h>
#include <rpos/features/location_provider/map.h>

namespace rpos { namespace system { namespace diagnosis { namespace serialization {

    RP_SLAMWARE_TEST_LOGGING_DECLARE_SERIALIZER(message::depth_camera::FlattenDepthCameraScanPoint);
    RP_SLAMWARE_TEST_LOGGING_DECLARE_SERIALIZER(rpos::features::location_provider::BitmapMap);

} } } }
