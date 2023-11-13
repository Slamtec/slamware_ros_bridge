/*
* application_context.h
* Application context (an standard implementation of IContext)
*
* Created by Tony Huang (tony@slamtec.com) at 2016-12-3
* Copyright 2016 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "i_context.h"
#include "service_dependency.h"

#include <rpos/core/rpos_core_config.h>

#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <map>
#include <set>
#include <tuple>

namespace rpos { namespace context {

    class RPOS_CORE_API ApplicationContext : public IContext {
    public:
        ApplicationContext();
        virtual ~ApplicationContext();

    public:
        template <class T>
        boost::shared_ptr<T> getService()
        {
            return boost::dynamic_pointer_cast<T>(getService(typeid(T)));
        }
        virtual IServicePtr getService(const std::string& serviceId);
        virtual IServicePtr getService(const std::type_index& serviceType);
        virtual std::vector<IServicePtr> getServices();
        virtual std::vector<IServicePtr> getServices(const std::type_index& serviceType);

        virtual IContext& addService(IServicePtr service);
        virtual IContext& removeService(IServicePtr service);
        virtual IContext& clearServices();

    public:
        virtual ApplicationContext& defineModule(const std::string& name, const std::vector<std::string>& prefixes);

    public:
        virtual bool startService(const std::string& serviceId);
        virtual bool startService(IServicePtr service);
        virtual bool startServices(const std::type_index& serviceType);
        virtual bool startServices();

        virtual bool stopService(IServicePtr service);
        virtual bool stopService(const std::string& serviceId);
        virtual void stopServices(const std::type_index& serviceType);
        virtual void stopServices();

    public:
        virtual std::string generateDotGraph();

    private:
        void assignServiceId_(IServicePtr service);
        void removeService_(IServicePtr service);
        bool startService_(IServicePtr service);
        bool stopService_(IServicePtr service);
        void stopServices_();
        bool resolveDependencies_(IServicePtr service);
        bool sortByDependency_(std::vector<IServicePtr>& services);
        std::tuple<std::string, std::string> resolveModule_(const std::string& name);
        void generateModuleSubgraph_(
            std::stringstream& ss,
            std::set<std::string>& unresolvedTypes,
            size_t clusterId,
            const std::string& modName
        );
        void generateWire_(
            std::stringstream& ss,
            std::set<std::string>& unresolvedTypes,
            const std::string& indent,
            const std::string& from,
            rpos::context::IServiceDependency* dep,
            const std::string& modName,
            bool onlyIncludeModuleWires = false
        );

    private:
        mutable boost::mutex lock_;

        boost::unordered_set<IServicePtr> services_;
        boost::unordered_map<std::string, IServicePtr> nameServiceMap_;
        boost::unordered_multimap<std::string, IServicePtr> typeServiceMap_;
        boost::unordered_multimap<IServicePtr, IServicePtr> dependencyReverseMap_;
        std::map<std::string, std::vector<std::string>> modules_;
        std::map<std::string, size_t> moduleIdMap_;
    };

} }
