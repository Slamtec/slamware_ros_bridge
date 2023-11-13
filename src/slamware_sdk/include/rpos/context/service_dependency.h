/*
* service_dependency.h
* Implementation of service dependency
*
* Created by Tony Huang (tony@slamtec.com) at 2016-12-3
* Copyright 2016 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "i_service.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/core/demangle.hpp>
#include <boost/noncopyable.hpp>

namespace rpos { namespace context {

    enum ServiceDependencyFlag {
        ServiceDependencyFlagRequired = 0,
        ServiceDependencyFlagOptional = 1,
        ServiceDependencyFlagDefault = ServiceDependencyFlagRequired
    };

    template < class TService >
    class ServiceDependency : public IServiceDependency, private boost::noncopyable {
    public:
        ServiceDependency(const std::string& name, ServiceDependencyFlag dependencyFlag = ServiceDependencyFlagDefault)
            : name_(name), flag_(dependencyFlag)
        {}

        ~ServiceDependency()
        {}

        std::string getName() const
        {
            boost::lock_guard<boost::mutex> guard(lock_);
            return name_;
        }

        std::type_index getType() const
        {
            return std::type_index(typeid(TService));
        }

        virtual bool isOptional() const
        {
            return (ServiceDependencyFlagOptional == (ServiceDependencyFlagOptional & flag_));
        }

        boost::shared_ptr<IService> getService() const
        {
            boost::lock_guard<boost::mutex> guard(lock_);
            return service_;
        }

        boost::shared_ptr<TService> getTypedService()
        {
            return boost::dynamic_pointer_cast<TService>(getService());
        }

        void setService(boost::shared_ptr<IService> service)
        {
            boost::lock_guard<boost::mutex> guard(lock_);

            auto serviceOfType = boost::dynamic_pointer_cast<TService>(service);

            if (service && !serviceOfType)
                throw std::runtime_error("Service instance cannot be casted to " + boost::core::demangle(typeid(TService).name()) + " for " + name_);

            service_ = service;
        }

    public:
        TService& operator*()
        {
            boost::lock_guard<boost::mutex> guard(lock_);

            return *boost::dynamic_pointer_cast<TService>(service_);
        }

        const TService& operator*() const
        {
            boost::lock_guard<boost::mutex> guard(lock_);

            return *boost::dynamic_pointer_cast<TService>(service_);
        }

        TService* operator->()
        {
            boost::lock_guard<boost::mutex> guard(lock_);
            return dynamic_cast<TService*>(service_.get());
        }

        const TService* operator->() const
        {
            boost::lock_guard<boost::mutex> guard(lock_);
            return dynamic_cast<const TService*>(service_.get());
        }

        operator bool() const
        {
            boost::lock_guard<boost::mutex> guard(lock_);
            return (bool)service_;
        }

    private:
        mutable boost::mutex lock_;

        std::string name_;
        boost::shared_ptr<IService> service_;
        ServiceDependencyFlag flag_;
    };
    
    template <typename TService>
    boost::shared_ptr<TService> to_shared_ptr(ServiceDependency<TService>& dep) {
        return boost::dynamic_pointer_cast<TService>(dep.getService());
    }
} }
