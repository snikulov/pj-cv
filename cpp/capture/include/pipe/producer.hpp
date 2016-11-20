#ifndef PRODUCER_HPP__
#define PRODUCER_HPP__

//#include <pipe/filter.hpp>
#include "monitor_queue.hpp"

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

template <class T>
class filter
{
public:
    virtual ~filter()
    {
        parent_.reset();
    }

    virtual void operator()() = 0;

    void set_prev(std::shared_ptr<filter<T> > p)
    {
        parent_ = p;
    }

    T get()
    {
        return parent_->get();
    }

    void put(T t)
    {
        q_.enqueue(t);
    }

    monitor_queue<T> q_;
    std::shared_ptr<filter<T> > parent_;
};

template <class T>
class producer : public filter<T>
{
public:
    producer(std::function<T(void)> f, std::function<bool(const T&)> pred, std::function<bool(void)> s)
        : func_(f)
        , pred_(pred)
        , stop_(s)
        , lg_(Logger::getInstance("producer"))
    {
        LOG4CPLUS_INFO(lg_, "Frame producer started...");
    }

    void operator()()
    {
        while (!stop_())
        {
            T d{ func_() };
            // will go further only if condition pass
            if (pred_(d))
            {
                LOG4CPLUS_TRACE(lg_, "put frame into queue");
                put(d);
            }
            else
            {
                LOG4CPLUS_WARN(lg_, "frame check does not pass");
            }
        }
    }

    std::function<T(void)> func_;
    std::function<bool(const T&)> pred_;
    std::function<bool(void)> stop_;

    log4cplus::Logger lg_;
};

template <class T>
class transformer : public filter<T>
{
public:
    transformer(std::function<T(T)> f, std::function<bool(const T&)> pred, std::function<bool(void)> s)
        : func_(f)
        , pred_(pred)
        , stop_(s)
    {
    }

    void operator()()
    {
        while (!stop_())
        {
            T d{ func_(get()) };
            // will go further, only if condition pass
            if (pred_(d))
            {
                put(d);
            }
        }
    }

    std::function<T(T)> func_;
    std::function<bool(const T&)> pred_;
    std::function<bool(void)> stop_;
};

template <class T>
class sink : public filter<T>
{
public:
    sink(std::function<void(T)> f, std::function<bool(void)> s)
        : func_(f)
        , stop_(s)
    {
    }

    void operator()()
    {
        while (!stop_())
        {
            func_(this->get());
        }
    }

    std::function<void(T)> func_;
    std::function<bool(void)> stop_;
};

#endif // PRODUCER_HPP__
