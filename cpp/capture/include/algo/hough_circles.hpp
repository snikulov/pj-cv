#ifndef HOUGH_CIRCLES_HPP__
#define HOUGH_CIRCLES_HPP__
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


#include "algo/od_interface.hpp"
#include <opencv2/opencv.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>


using namespace log4cplus;
using namespace log4cplus::helpers;

class hough_circles : public od_interface
{
public:
    hough_circles()
        : lg_(Logger::getInstance("hough_circles"))
        , num_received_(0)
        , num_detected_(0)
    {
        LOG4CPLUS_INFO(lg_, "Running hough_circles for initial detection");
    }

    bool has_objects(opencv_frame_t& d)
    {
        cv::Mat gray;
        std::vector<cv::Vec3f> circles;

        cv::cvtColor(*(d.frame_), gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, gray, { 5, 5 }, 1.5, 1.5);
        cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 2, 300, 50, 300, 350, 650);

        if (!circles.empty())
        {
            d.circles_.reset(new std::vector<cv::Vec3f>(circles.size()));
            d.circles_->swap(circles);
//            std::swap(*(d.circles_), circles);
            return true;
        }
        return false;
    }

    opencv_frame_t operator()(const opencv_frame_t& d)
    {
        opencv_frame_t tmp = d;
        if (tmp.frame_ && !tmp.frame_->empty())
        {
            num_received_++;
            if (has_objects(tmp))
            {
                num_detected_++;
//                LOG4CPLUS_INFO(lg_, "detected circles in frame");
            }
        }

        if (num_received_ > 0 && !(num_received_ % 1000))
        {
            LOG4CPLUS_INFO(lg_, "stat: num_received_=" << num_received_ << " num_detected_=" << num_detected_);
        }

        return tmp;
    }

    log4cplus::Logger lg_;
    unsigned long long num_received_;
    unsigned long long num_detected_;

};

#endif // HOUGH_CIRCLES_HPP__
