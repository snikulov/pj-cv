#ifndef JPEG_WRITTER_HPP__
#define JPEG_WRITTER_HPP__
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


#include <sstream>
#include <date.h>

#include <opencv2/opencv.hpp>

#include <boost/application.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include "opencv_frame.hpp"
#include "monitor_queue.hpp"
#include "algo/od_interface.hpp"

class jpeg_writter
{
public:
    jpeg_writter(boost::application::context& ctx, const boost::filesystem::path& p,
                 monitor_queue<opencv_frame_t>& q, std::shared_ptr<od_interface> det)
        : context_(ctx)
        , path_(p)
        , queue_(q)
        , detector_(det)
    {
    }

    void operator()()
    {
        auto st = context_.find<boost::application::status>();

        opencv_frame_t data;

        while (st->state() != boost::application::status::stopped)
        {
            data = queue_.dequeue();
            if (data.frame_)
            {
                if (detector_->has_objects(data))
                {
                    if (write_image(data))
                    {
                        std::cerr << "Writted image: " << get_fname(data) << std::endl;
                    }
                }
            }
        }
    }

private:
    jpeg_writter() = delete;

    bool write_image(const opencv_frame_t& d)
    {
        boost::filesystem::path f{ path_ };
        f /= get_fname(d);
        return cv::imwrite(f.string(), *(d.frame_));
    }

    std::string get_fname(const opencv_frame_t& d)
    {
        using namespace date;
        std::ostringstream ss;
        ss << d.time_captured_ << "-" << std::this_thread::get_id();
        std::string ret{ ss.str() };

        auto pred = [](char& ch)
        { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
        std::replace_if(ret.begin(), ret.end(), pred, '-');
        ret += ".jpg";
        return ret;
    }

    boost::application::context& context_;
    boost::filesystem::path path_;
    monitor_queue<opencv_frame_t>& queue_;
    std::shared_ptr<od_interface> detector_;
};

#endif
