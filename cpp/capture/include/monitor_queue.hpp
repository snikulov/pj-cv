#ifndef MONITOR_QUEUE_HPP__
#define MONITOR_QUEUE_HPP__
////////////////////////////////////////////////////////////////////////////////////////////////
///	Copyright (C) 2016-2017, Sergei Nikulov (sergey.nikulov@gmail.com)
///	All rights reserved.
///
///	Redistribution and use in source and binary forms, with or without
///	modification, are permitted provided that the following conditions are met:
///	* Redistributions of source code must retain the above copyright
///	notice, this list of conditions and the following disclaimer.
///	* Redistributions in binary form must reproduce the above copyright
///	notice, this list of conditions and the following disclaimer in the
///	documentation and/or other materials provided with the distribution.
///	* Neither the name of the <organization> nor the
///	names of its contributors may be used to endorse or promote products
///	derived from this software without specific prior written permission.
///
///	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
///	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
///	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
///	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
///	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
///	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
///	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////

//
// Sample generic monitor queue for data sink
//

#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <boost/noncopyable.hpp>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>


using namespace log4cplus;
using namespace log4cplus::helpers;

template <typename T>
class monitor_queue
{
public:
    monitor_queue(std::size_t maxelm = 1000)
        : max_size_(maxelm)
        , lg_(Logger::getInstance("monitorq"))
    {
    }

    bool enqueue(T d)
    {
        std::lock_guard<std::mutex> lock(mx_);
        bool ret_val = true;
        if (q_.size() >= max_size_)
        {
            LOG4CPLUS_WARN(lg_, "Queue exceeded " << max_size_ << " Will drop frames...");
            // drop frame
            q_.pop();
            ret_val = false;
        }
        q_.push(d);
        cond_.notify_all();
        return ret_val;
    }

    T dequeue(int ms_delay = 1000)
    {
        std::unique_lock<std::mutex> lk(mx_);
        if (q_.empty())
        {
            if (ms_delay > 0)
            {
                cond_.wait_for(lk, std::chrono::milliseconds(ms_delay));
            }
            else if (ms_delay < 0)
            {
                // infinite
                cond_.wait(lk);
            }
            // else no delay
        }

        T ret_val;

        if (!q_.empty())
        {
            ret_val = q_.front();
            q_.pop();
        }
        return ret_val;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mx_);
        return q_.empty();
    }

private:
    std::size_t max_size_;
    std::mutex mx_;
    std::condition_variable cond_;
    std::queue<T> q_;

    log4cplus::Logger lg_;
};
#endif
