//
// Sample generic monitor queue for data sink
//
#ifndef MONITOR_QUEUE_HPP__
#define MONITOR_QUEUE_HPP__

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
