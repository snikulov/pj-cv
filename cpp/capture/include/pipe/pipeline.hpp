#ifndef PIPELINE_HPP__
#define PIPELINE_HPP__
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


#include <list>
#include <memory>
#include <thread>

#include "producer.hpp"

template <class T, class D>
class pipeline
{
public:
    pipeline()
        : is_running_(false)
    {
    }

    ~pipeline()
    {
        if (is_running_)
        {
            for (auto& elm : tgroup_)
            {
                elm->join();
            }
        }
        qlist_.clear();
    }

    void add_filter(T f)
    {
        if (!filters_.empty())
        {
            auto glue = std::make_shared<monitor_queue<D> >();
            T& prev_step = filters_.back();
            prev_step->output_ = glue;
            f->input_ = glue;

            // for debug
            qlist_.push_back(glue);
        }
        filters_.push_back(f);
    }

    bool run()
    {
        if ((!is_running_) && (!filters_.empty()))
        {
            is_running_ = true;
            for (auto it = filters_.rbegin(); it != filters_.rend(); ++it)
            {

                auto elm = *it;
                tgroup_.push_back(std::make_shared<std::thread>(std::ref(*elm)));
#if 0
                // some magic
                auto nptr = elm.get();
                auto prodptr = dynamic_cast<producer<T>*>(nptr);
                if (prodptr)
                {
                    tgroup_.push_back(std::make_shared<std::thread>(std::ref(*prodptr)));
                }
                else
                {
                    auto transptr = dynamic_cast<transformer<T>*>(nptr);
                    if (transptr)
                    {
                        tgroup_.push_back(std::make_shared<std::thread>(std::ref(*transptr)));
                    }
                    else
                    {
                        auto finptr = dynamic_cast<sink<T>*>(nptr);
                        if (finptr)
                        {
                            tgroup_.push_back(std::make_shared<std::thread>(std::ref(*finptr)));
                        }
                        else
                        {
                            throw std::runtime_error("Unable cast to filter!");
                        }
                    }
                }
#endif
            }
        }
        return is_running_;
    }

    /// no copy
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

private:
    std::list<T> filters_;
    bool is_running_;
    std::list<std::shared_ptr<std::thread> > tgroup_;

    // for debug
    std::list<std::shared_ptr<monitor_queue<D> > > qlist_;
};

#endif // PIPELINE_HPP__
