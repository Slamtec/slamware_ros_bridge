/*
* base_service.h
* Bare service implementation for IService interface
*
* Created by Tony Huang (tony@slamtec.com) at 2016-12-3
* Copyright 2016 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "i_service.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/core/demangle.hpp>
#include <rpos/system/util/log.h>
#include <rpos/system/util/string_utils.h>

namespace rpos { namespace context {

    RPOS_CORE_API std::string to_string(const std::type_index& type);

    template < class ServiceType >
    class BaseService : public IService {
    protected:
        BaseService()
            : status(ServiceStatusStopped)
            , logger(getType_())
        {}

        virtual ~BaseService()
        {}

    protected:
        virtual bool onStart() = 0;
        virtual bool onStop() = 0;

    public:
        virtual std::string getId() const
        {
            boost::lock_guard<boost::mutex> guard(lock);
            return id_;
        }

        virtual void setId(const std::string& id)
        {
            boost::lock_guard<boost::mutex> guard(lock);
            id_ = id;
            if (rpos::system::util::starts_with(id, getType_()))
                logger.setSource(id);
            else
                logger.setSource(getType_() + "#" + id);
        }

        virtual std::string getType() const
        {
            return getType_();
        }

    public:
        virtual const std::vector<std::type_index>& getProvides() const
        {
            boost::lock_guard<boost::mutex> guard(lock);
            return provides_;
        }

        virtual const std::vector<IServiceDependency*>& getDependencies()
        {
            boost::lock_guard<boost::mutex> guard(lock);
            return depends_;
        }

    public:
        virtual ServiceStatus getStatus() const
        {
            boost::lock_guard<boost::mutex> guard(lock);
            return status;
        }

        virtual bool start()
        {
            boost::lock_guard<boost::mutex> guard(lock);

            if (status == ServiceStatusRunning)
            {
                return true;
            }
            else if (status == ServiceStatusStartingUp || status == ServiceStatusStopping)
            {
                throw std::runtime_error("Service is starting up or stopping, shouldn't be in this branch");
            }
            else if (status == ServiceStatusStopped)
            {
                status = ServiceStatusStartingUp;
                logger.info_out("Starting up service...");
                if (!onStart())
                {
                    status = ServiceStatusStopped;
                    logger.error_out("Failed to start service");
                    return false;
                }
                else
                {
                    status = ServiceStatusRunning;
                    logger.info_out("Successfully start service");
                    return true;
                }
            }
            else
            {
                throw std::runtime_error("Unexpected branch");
            }
            return false;
        }

        virtual bool stop()
        {
            boost::lock_guard<boost::mutex> guard(lock);

            if (status == ServiceStatusStopped)
            {
                return true;
            }
            else if (status == ServiceStatusStartingUp || status == ServiceStatusStopping)
            {
                throw std::runtime_error("Service is starting up or stopping, shouldn't be in this branch");
            }
            else if (status == ServiceStatusRunning)
            {
                status = ServiceStatusStopping;
                logger.info_out("Shutting down service...");
                if (!onStop())
                {
                    status = ServiceStatusRunning;
                    logger.error_out("Failed to stop service");
                    return false;
                }
                else
                {
                    status = ServiceStatusStopped;
                    logger.info_out("Successfully stopped service");
                    return true;
                }
            }
            else
            {
                throw std::runtime_error("Unexpected branch");
            }
            return false;
        }

    protected:
        // dependency management
        void depends(IServiceDependency* depend)
        {
            boost::lock_guard<boost::mutex> guard(lock);

            if (status != ServiceStatusStopped)
                throw std::runtime_error("Depends can only be modified at stopped status");

            depends_.push_back(depend);
        }

        void provides(const std::type_index& type)
        {
            boost::lock_guard<boost::mutex> guard(lock);
            
            if (status != ServiceStatusStopped)
                throw std::runtime_error("Provides can only be modified at stopped status");

            provides_.push_back(type);
        }

        template < class TInterface >
        void provides()
        {
            provides(std::type_index(typeid(TInterface)));
        }

        void clearDepends()
        {
            boost::lock_guard<boost::mutex> guard(lock);
            depends_.clear();
        }

        void clearProvides()
        {
            boost::lock_guard<boost::mutex> guard(lock);
            provides_.clear();
        }

    private:
        static std::string getType_()
        {
            return to_string(typeid(ServiceType));
        }

    protected:
        mutable boost::mutex lock;
        ServiceStatus status;
        mutable rpos::system::util::LogScope logger;

    private:
        std::string id_;
        std::vector<std::type_index> provides_;
        std::vector<IServiceDependency*> depends_;
    };

} }
