#ifndef JPEG_WRITTER_EX_HPP
#define JPEG_WRITTER_EX_HPP
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

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

class jpeg_writter_ex
{
public:
    jpeg_writter_ex(const std::string& p)
        : lg_(Logger::getInstance("sink"))
        , path_(p)
    {
        namespace fs = boost::filesystem;

        if (!fs::exists(path_))
        {
            if (!fs::create_directories(path_))
            {
                LOG4CPLUS_FATAL(lg_, "Unable to create folder for result: " << path_.string());
                throw std::runtime_error("Unable create output folder");
            }
        }

        LOG4CPLUS_INFO(lg_, "Result will be stored in " << fs::absolute(path_).string());
    }

    void operator()(opencv_frame_t d)
    {
        if (d.frame_ && !d.frame_->empty())
        {
            if (d.squares_ && !(d.squares_->empty()))
            {
                // draw_objects(d);
                // detected image
                std::string fname = get_fname(d);
                if (write_image(d, fname))
                {
                    LOG4CPLUS_INFO(lg_, "Writted image: " << fname);
                }
                else
                {
                    LOG4CPLUS_WARN(lg_, "Error writting fail: " << fname);
                }
            }
        }
    }

private:

    void draw_objects(opencv_frame_t& d)
    {
        if (d.circles_ && !(d.circles_->empty()))
        {
            // draw circles
            const auto& ci = *(d.circles_);
            for (const auto& elm : ci)
            {
                cv::Point center(cvRound(elm[0]), cvRound(elm[1]));
                int radius = cvRound(elm[2]);
                circle(*(d.frame_), center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
                // draw the circle outline
                circle(*(d.frame_), center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
            }
        }

        if (d.squares_ && !(d.squares_->empty()))
        {
            const auto& sq = *(d.squares_);
            for (const auto& elm : sq)
            {
                cv::rectangle(*(d.frame_), elm, cv::Scalar(0, 0, 255), 4);
            }
        }
    }

    bool write_image(const opencv_frame_t& d, const std::string& fname)
    {
        boost::filesystem::path f{ path_ };
        f /= fname;
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

    log4cplus::Logger lg_;
    boost::filesystem::path path_;
};

#endif // JPEG_WRITTER_EX_HPP
