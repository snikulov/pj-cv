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


#include "pipe/pipeline.hpp"
#include "pipe/producer.hpp"
#include "opencv_frame.hpp"
#include "algo/hough_circles.hpp"
#include "algo/hog_dlib.hpp"
#include "jpeg_writter_ex.hpp"
#include "jpeg_writter_sql.hpp"

#include <opencv2/opencv.hpp>
#include <boost/application.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

namespace app = boost::application;
namespace fs = boost::filesystem;

using namespace log4cplus;
using namespace log4cplus::helpers;


class ocv_cam
{
public:
    ocv_cam(std::string& url)
        : url_(url)
        , cap_(new cv::VideoCapture)
        , err_count_(0)
        , clog_(Logger::getInstance("camera"))
        , frames_count_(0)
        , call_count_(0)
        , start_time_(std::chrono::high_resolution_clock::now())
        , fps_(0.0)
    {
        if (!cap_->open(url_))
        {
            LOG4CPLUS_FATAL(clog_, "Unable to open " << url_);
            throw std::runtime_error("Unable to open " + url_);
        }
        LOG4CPLUS_INFO(clog_, "Running capture on: " << url_);
    }

    opencv_frame_t operator()()
    {
        opencv_frame_t frame{};
        call_count_++;
        // take each 10 frame
        frame.frame_ = std::make_shared<cv::Mat>();
        if (!cap_->read(*(frame.frame_)))
        {
            err_count_++;
            if (err_count_ > 10)
            {
                LOG4CPLUS_WARN(clog_, "Number of empty frames " << err_count_ << ". Try re-open");
                cap_.reset(new cv::VideoCapture());
                if (!cap_->open(url_))
                {
                    LOG4CPLUS_FATAL(clog_, "Unable to re-open " << url_ << " after " << err_count_ << " retries");
                    throw std::runtime_error("Unable to open " + url_);
                }
                else
                {
                    err_count_    = 0;
                    frames_count_ = 0;
                    LOG4CPLUS_INFO(clog_, "Re-opened capture...");
                }
            }
        }
        else
        {
            frame.time_captured_ = std::chrono::system_clock::now();
            err_count_ = 0;
            frames_count_++;

            if (!(frames_count_ % 25)) {
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time_);
                auto cnt = diff.count();
                if (cnt > 0 && !(cnt % 5))
                {
                    LOG4CPLUS_INFO(clog_, "frames received: " << frames_count_);
                }
            }
        }
        // will rate-limit output from 25->5
        if(!(call_count_%5))
        {
            return frame;
        }
        return opencv_frame_t{};
    }

private:

    void update_fps()
    {
        //using namespace std::chrono_literals;
        //using namespace std::literals::chrono_literals;
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time_);

        const auto CINTERVAL = std::chrono::seconds(2);
        if (diff > CINTERVAL)
        {
            fps_ = frames_count_ / diff.count();
        }
    }

    std::string url_;
    std::unique_ptr<cv::VideoCapture> cap_;
    unsigned int err_count_;
    log4cplus::Logger clog_;

    // processed frames count
    unsigned long long frames_count_;
    unsigned long long call_count_;
    // start time
    std::chrono::high_resolution_clock::time_point start_time_;

    double fps_;
};

class worker
{
public:
    explicit worker(app::context& ctx)
        : ctx_(ctx)
    {
    }

    int operator()()
    {
        auto ctxarg = ctx_.find<app::args>();
        namespace po = boost::program_options;

        std::string url = "rtsp://admin:admin@212.45.31.140:554/h264";
        std::string dst = "192.168.9.233";
        std::string svmpath = "tr2_80x80.svm";

        po::options_description desc{ "Options" };
        // clang-format off
        desc.add_options()
            ("help,h", "Help")
            ("url,u", po::value<std::string>(&url)->default_value("rtsp://admin:admin@212.45.31.140:554/h264"), "camera url")
            ("out,o", po::value<std::string>(&dst)->default_value("192.168.9.233"), "db server ip")
            ("svmpath,s", po::value<std::string>(&svmpath)->default_value("tr2_80x80.svm"), "path to obj detector svm");
        // clang-format on
        po::variables_map vm;
        po::store(po::parse_command_line(ctxarg->argc(), ctxarg->argv(), desc), vm);
        po::notify(vm);

        if (!vm.count("help"))
        {
            auto cam = std::make_shared<ocv_cam>(url);

            auto is_stopped = [&]()->bool
            {
                return (ctx_.find<app::status>())->state() == app::status::stopped;
            };

            auto frame_not_null = [](const opencv_frame_t& data) -> bool {
                return data.frame_ && (!data.frame_->empty());
            };

            pipeline<std::shared_ptr<filter<opencv_frame_t> >, opencv_frame_t> pipe;
            auto prod = std::make_shared<producer<opencv_frame_t> >(std::ref(*cam), frame_not_null, is_stopped);
            pipe.add_filter(prod);

            auto circles_found = [frame_not_null](const opencv_frame_t& data) -> bool {
                return frame_not_null(data) && data.circles_ && (!data.circles_->empty());
            };

            auto circ = std::make_shared<hough_circles>();
            auto step1 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*circ), circles_found, is_stopped);
            pipe.add_filter(step1);

            auto h = std::make_shared<hog>(svmpath);
            auto step2 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*h), circles_found, is_stopped);
            pipe.add_filter(step2);

            auto jw = std::make_shared<jpeg_writter_sql>(dst);
            auto step3 = std::make_shared<sink<opencv_frame_t> >(std::ref(*jw), is_stopped);
            pipe.add_filter(step3);

            pipe.run();

            ctx_.find<app::wait_for_termination_request>()->wait();
        }
        else
        {
            std::cout << desc << "\n";
        }
        return 0;
    }

private:
    app::context& ctx_;
};

static void init_logger()
{
    using namespace log4cplus;
    using namespace log4cplus::helpers;
    try
    {
        PropertyConfigurator::doConfigure("log4cplus.properties");
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(0);
    }

}

int main(int argc, char** argv)
{
    init_logger();

    auto lg = Logger::getInstance("main");
    LOG4CPLUS_INFO(lg, "pipeline v0.1");
    LOG4CPLUS_INFO(lg, "Running on " << std::thread::hardware_concurrency() << " HW cores");
    LOG4CPLUS_INFO(lg, "Press <Ctrl+C> to exit...");

    app::context app_context;
    app_context.insert<app::args>(
        app::csbl::make_shared<app::args>(argc, argv));

    worker app(app_context);

    return app::launch<app::common>(app, app_context);

    return 0;
}
