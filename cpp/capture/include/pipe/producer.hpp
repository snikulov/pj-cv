#ifndef PRODUCER_HPP__
#define PRODUCER_HPP__
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
    filter()
    {
    }

    virtual ~filter()
    {
        input_.reset();
        output_.reset();
    }

    virtual void operator()() = 0;

    T get()
    {
        if (input_)
        {
            return input_->dequeue();
        }
        return T();
    }

    void put(T t)
    {
        if (output_)
        {
            output_->enqueue(t);
        }
    }

    std::shared_ptr<monitor_queue<T> > input_;
    std::shared_ptr<monitor_queue<T> > output_;
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
            T d(func_());
            // will go further only if condition pass
            if (pred_(d))
            {
                LOG4CPLUS_TRACE(lg_, "put frame into queue");
                this->put(d);
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
            T d(func_(this->get()));
            // will go further, only if condition pass
            if (pred_(d))
            {
                this->put(d);
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
