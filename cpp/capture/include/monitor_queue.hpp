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

template <typename T>
class monitor_queue : private boost::noncopyable
{
public:
    monitor_queue(std::size_t max_size = 100)
        : max_size_(max_size)
    {
    }

    void enqueue(T d)
    {
        std::lock_guard<std::mutex> lock(mx_);
        if (q_.size() >= max_size_)
        {
            // drop frame
            q_.pop();
        }
        q_.push(d);
        cond_.notify_all();
    }

    T dequeue(int ms_delay = 500)
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
};
#endif
