/**
* system_parameters.h
* The System Parameters
*
* Created By  @ 2016-4-19
* Copyright (c) 2016 Shanghai SlamTec Co., Ltd.
*/

#pragma once

#define SYSPARAM_ROBOT_SPEED "base.max_moving_speed"
#define SYSPARAM_ROBOT_ANGULAR_SPEED "base.max_angular_speed"
#define SYSVAL_ROBOT_SPEED_HIGH "high"
#define SYSVAL_ROBOT_SPEED_MEDIUM "medium"
#define SYSVAL_ROBOT_SPEED_LOW "low"

#define SYSPARAM_POWEROFF_WAIT_TIME           "base.power_off_wait_time"

#define SYSPARAM_EMERGENCY_STOP               "base.emergency_stop"
#define SYSVAL_EMERGENCY_STOP_ON   "on"
#define SYSVAL_EMERGENCY_STOP_OFF  "off"

#define SYSPARAM_BRAKE_RELEASE                "base.brake_release"
#define SYSVAL_BRAKE_RELEASE_ON    "on"
#define SYSVAL_BRAKE_RELEASE_OFF   "off"

#define SYSPARAM_DOCKED_REGISTER_STRATEGY "docking.docked_register_strategy"
#define SYSVAL_DOCKED_REGISTER_STRATEGY_ALWAYS "always"
#define SYSVAL_DOCKED_REGISTER_STRATEGY_WHEN_NOT_EXISTS "when_not_exists"

#define SYSPARAM_ACTIONS_WAIT_FOR_ACCEPTABLE_PATH_TIME      "actions.wait_for_acceptable_path_time"

#define SYSPARAM_VIRTUAL_TRACKS_ACCESSIBLE_AREA     "virtual_track.enable_limited_accessible_area"
#define SYSVAL_ACCESSIBLE_AREA_ON    "on"
#define SYSVAL_ACCESSIBLE_AREA_OFF   "off"