#pragma once

#include <rpos/core/rpos_core_config.h>
#include <string>
#include <vector>
#include <cstdint>

namespace rpos { namespace system { namespace util {

    struct EccDasResult
    {
        bool res;
        std::string errMsg;
        EccDasResult() : res(true), errMsg("") {}
        EccDasResult(bool res, const std::string& errMsg) : res(res), errMsg(errMsg) {}
    };

    RPOS_CORE_API EccDasResult EccDSASignature(bool isFile, const std::string& priKey, const std::string& oriMsg, std::vector<std::uint8_t>& signMsg);

    RPOS_CORE_API EccDasResult EccDASVerifySignature(bool isFile, const std::string& pubKey, const std::string& oriMsg, const std::vector<std::uint8_t>& signMsg);

}}}