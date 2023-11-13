/*
* i_service.h
* IService is an abstract interface of elements in the container
*
* Created by Tony Huang (tony@slamtec.com) at 2016-12-3
* Copyright 2016 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include <string>
#include <vector>
#include <typeindex>
#include <boost/shared_ptr.hpp>

namespace rpos { namespace context {

    enum ServiceStatus {
        ServiceStatusStopped,
        ServiceStatusStartingUp,
        ServiceStatusRunning,
        ServiceStatusStopping
    };

    class IService;

    class IServiceDependency {
    public:
        virtual ~IServiceDependency() {}
        virtual std::string getName() const = 0;
        virtual std::type_index getType() const = 0;
        virtual bool isOptional() const = 0;
        virtual boost::shared_ptr<IService> getService() const = 0;
        virtual void setService(boost::shared_ptr<IService> service) = 0;
    };

    class IService {
    public:
        // basic info
        virtual ~IService() {}
        virtual std::string getId() const = 0;
        virtual void setId(const std::string&) = 0;

        virtual std::string getType() const = 0;

    public:
        // dependency
        virtual const std::vector<std::type_index>& getProvides() const = 0;
        virtual const std::vector<IServiceDependency*>& getDependencies() = 0;

    public:
        // service management
        virtual ServiceStatus getStatus() const = 0;
        virtual bool start() = 0;
        virtual bool stop() = 0;
    };

} }
