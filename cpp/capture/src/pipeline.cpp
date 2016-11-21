#include "pipe/pipeline.hpp"
#include "pipe/producer.hpp"
#include "opencv_frame.hpp"
#include "algo/hough_circles.hpp"
#include "algo/hog_dlib.hpp"
#include "jpeg_writter_ex.hpp"

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
        return frame;
    }

private:

    void update_fps()
    {
        //using namespace std::chrono_literals;
        using namespace std::literals::chrono_literals;
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time_);

        if (diff > 2s)
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
        std::string url{ "rtsp://admin:admin@212.45.31.140:554/h264" };
        auto cam = std::make_shared<ocv_cam>(url);
        auto is_stopped = [&]()->bool
        {
            return (ctx_.find<app::status>())->state() == app::status::stopped;
        };

        auto frame_not_null = [](const opencv_frame_t& data) -> bool {
            return data.frame_ && (!data.frame_->empty());
        };

        pipeline<std::shared_ptr<filter<opencv_frame_t> > , opencv_frame_t> pipe;
        auto prod = std::make_shared<producer<opencv_frame_t> >(std::ref(*cam), frame_not_null, is_stopped);
        pipe.add_filter(prod);

        auto circles_found = [frame_not_null](const opencv_frame_t& data) -> bool {
            return frame_not_null(data) && data.circles_ && (!data.circles_->empty());
        };

        auto circ = std::make_shared<hough_circles>();
        auto step1 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*circ), circles_found, is_stopped);
        pipe.add_filter(step1);

        std::string svmpath = "tr2_80x80.svm";
        auto h = std::make_shared<hog>(svmpath);
        auto step2 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*h), circles_found, is_stopped);
        pipe.add_filter(step2);

        std::string imgpath = "images";
        auto jw = std::make_shared<jpeg_writter_ex>(imgpath);
        auto step3 = std::make_shared<sink<opencv_frame_t> >(std::ref(*jw), is_stopped);
        pipe.add_filter(step3);

        pipe.run();

        ctx_.find<app::wait_for_termination_request>()->wait();

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
