#ifndef HOG_DLIB_HPP__
#define HOG_DLIB_HPP__
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

#include <dlib/opencv/cv_image.h>
#include <dlib/svm_threaded.h>
#include <dlib/image_processing.h>
#include <dlib/data_io.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

using img_scanner_t = dlib::scan_fhog_pyramid<dlib::pyramid_down<6> >;

class hog : public od_interface
{
public:
    explicit hog(const std::string& path_to_svm)
        : lg_(Logger::getInstance("hog"))
        , detector_(new dlib::object_detector<img_scanner_t>())
        , svm_path_(path_to_svm)
    {
        LOG4CPLUS_INFO(lg_, "Loading SVM from " << svm_path_);
        dlib::deserialize(svm_path_) >> *detector_;
        LOG4CPLUS_INFO(lg_, "Finished loading.");
    }

    bool has_objects(opencv_frame_t& d)
    {
        cv::Mat tmp = *d.frame_;
        dlib::cv_image<dlib::bgr_pixel> cimg(tmp);
        std::vector<dlib::rectangle> dets{ (*detector_)(cimg) };
        if (!dets.empty())
        {
            auto dsize = dets.size();
            d.squares_.reset(new std::vector<cv::Rect>(dsize));
            for (const auto& r : dets)
            {
                d.squares_->push_back(dlib_to_ocv(r));
            }
            LOG4CPLUS_INFO(lg_, "Detected " << dsize << " objects");
            return true;
        }
        return false;
    }

    opencv_frame_t operator()(opencv_frame_t d)
    {
        opencv_frame_t tmp = d;
        if (tmp.frame_ && !tmp.frame_->empty())
        {
            // prev step found circles
            if (tmp.circles_ && !(tmp.circles_->empty()))
            {
                (void)has_objects(tmp);
            }
        }
        return tmp;
    }

private:
    cv::Rect dlib_to_ocv(const dlib::rectangle& r)
    {
        cv::Rect ret_val(r.left(), r.top(), r.width(), r.height());
//        LOG4CPLUS_INFO(lg_, "Obj params: " << "dlib(" << r.left() << ", " << r.top() << ", " << r.right() << ", " << r.bottom() << ")");
//        LOG4CPLUS_INFO(lg_, "Obj params: " << "ocv(" << ret_val.x << ", " << ret_val.y << ", " << ret_val.height << ", " << ret_val.width << ")");
        return ret_val;
    }

    log4cplus::Logger lg_;

    std::unique_ptr<dlib::object_detector<img_scanner_t> > detector_;
    std::string svm_path_;

};

#endif // HOG_DLIB_HPP__
