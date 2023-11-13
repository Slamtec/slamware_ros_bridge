/*
* threaded_service.h
* Based on BaseService<T>, and using dedicate thread to provide service
*
* Created by Tony Huang (tony@slamtec.com) at 2017-6-6
* Copyright 2017 (c) Shanghai Slamtec Co., Ltd.
*/

#pragma once

#include "base_service.h"
#include <boost/thread.hpp>
#include <rpos/system/this_thread.h>
#include <rpos/system/util/string_utils.h>

namespace rpos { namespace context {

    template < class ServiceType >
    class ThreadedService : public BaseService<ServiceType> {
    protected:
        ThreadedService()
            : working_(false)
        {}

        virtual ~ThreadedService() {}

    protected:
        mutable boost::mutex lock_;
        bool working_;

    private:
        boost::thread workThread_;

    protected:
        virtual void worker() = 0;

    protected:
        /**
        * This callback is called before start when the onStart() method is called
        * This callback runs in the caller thread of onStart() calls with lock_ locked
        * If this call returns false, the start progress will fail
        */
        virtual bool beforeStart()
        {
            return true;
        }

        /**
        * This callback is called after working_ flash is set to false on onStop() calls
        * This callback runs in the caller thread of onStop() calls with lock_ locked
        * This callback is usually used to notify conditional_variable to make worker thread exit properly
        * This call shouldn't fail
        */
        virtual void afterStop()
        {
        }

        /**
        * This callback is called after onStop() calls, and the service is stopped
        * This callback runs in the caller thread of onStop() calls with lock_ locked
        * This callback is usually used to do some clean up
        * This call shouldn't fail
        */
        virtual void afterStopped()
        {
        }

    private:
        void workerWrapper_()
        {
            auto id = this->getId();

            if (!id.empty())
                rpos::system::this_thread::setCurrentThreadName(rpos::system::util::split(id, '.').back());

            worker();
        }

    protected:
        virtual bool onStart()
        {
            boost::lock_guard<boost::mutex> guard(lock_);

            if (working_)
                return true;

            if (!beforeStart())
                return false;

            if (workThread_.joinable())
                workThread_.join();

            working_ = true;
            workThread_ = boost::move(boost::thread(boost::bind(&ThreadedService::workerWrapper_, this)));

            return true;
        }

        virtual bool onStop()
        {
            {
                boost::lock_guard<boost::mutex> guard(lock_);

                if (!working_)
                    return false;

                working_ = false;

                afterStop();
            }

            if (workThread_.joinable())
                workThread_.join();

            {
                boost::lock_guard<boost::mutex> guard(lock_);
                afterStopped();
            }
            return true;
        }
    };

} }
