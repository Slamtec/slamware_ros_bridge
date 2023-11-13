/*
* pseudo_device_config.h
* Pseudo Device config
*
* Created by Tony Huang (tony@slamtec.com) at 2018-10-22
* Copyright 2018 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include <rp/slamware/utils/any2tcp_bridge.h>

#ifdef PSEUDO_DEVICE_DLL
#   ifdef PSEUDO_DEVICE_EXPORT
#       define PSEUDO_DEVICE_API RPOS_MODULE_EXPORT
#   else
#       define PSEUDO_DEVICE_API RPOS_MODULE_IMPORT
#   endif
#else
#	define PSEUDO_DEVICE_API
#endif

#if defined(_WIN32) && ! defined(PSEUDO_DEVICE_EXPORT)
#   ifdef PSEUDO_DEVICE_DLL
#       pragma comment(lib, "pseudo_device.lib")
#   else
#       pragma comment(lib, "libpseudo_device.lib")
#   endif
#endif
