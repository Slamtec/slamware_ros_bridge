/*
* i_context.h
* IContext represents an abstract interface of service container (like ApplicationContext in Spring framework)
*
* Created by Tony Huang (tony@slamtec.com) at 2016-12-3
* Copyright 2016 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <typeindex>
#include <typeinfo>

namespace rpos { namespace context {

    class IService;

    typedef boost::shared_ptr<IService> IServicePtr;

    class IContext
    {
    public:
        virtual ~IContext() {}
        virtual IServicePtr getService(const std::string& serviceId) = 0;
        virtual IServicePtr getService(const std::type_index& serviceType) = 0;
        virtual std::vector<IServicePtr> getServices() = 0;
        virtual std::vector<IServicePtr> getServices(const std::type_index& serviceType) = 0;

        template < class TService >
        inline boost::shared_ptr<TService> getService()
        {
            return boost::dynamic_pointer_cast<TService>(getService(typeid(TService)));
        }

        template < class TService >
        inline void resolveService(boost::shared_ptr<TService>& svc)
        {
            svc = getService<TService>();
        }

        virtual IContext& addService(IServicePtr service) = 0;
        virtual IContext& removeService(IServicePtr service) = 0;
        virtual IContext& clearServices() = 0;

    public:
        virtual bool startService(const std::string& serviceId) = 0;
        virtual bool startService(IServicePtr service) = 0;
        virtual bool startServices(const std::type_index& serviceType) = 0;
        virtual bool startServices() = 0;
        
        virtual bool stopService(IServicePtr service) = 0;
        virtual bool stopService(const std::string& serviceId) = 0;
        virtual void stopServices(const std::type_index& serviceType) = 0;
        virtual void stopServices() = 0;
    };

} }
