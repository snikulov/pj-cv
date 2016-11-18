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

namespace app = boost::application;

class ocv_cam
{
public:
    ocv_cam(std::string& url)
        : url_(url)
        , cap_(new cv::VideoCapture)
        , err_count_(0)
    {
        if (!cap_->open(url_))
        {
            throw std::runtime_error("Unable to open " + url_);
        }
    }

    opencv_frame_t operator()()
    {
        opencv_frame_t frame;
        frame.frame_ = std::make_shared<cv::Mat>();
        if (!cap_->read(*(frame.frame_)))
        {
            err_count_++;
            if (err_count_ > 10)
            {
                cap_.reset(new cv::VideoCapture());
                if (!cap_->open(url_))
                {
                    throw std::runtime_error("Unable to open " + url_);
                }
                else
                {
                    err_count_ = 0;
                }
            }
        }
        else
        {
            err_count_ = 0;
        }
        frame.time_captured_ = std::chrono::system_clock::now();
        return frame;
    }

private:
    std::string url_;
    std::unique_ptr<cv::VideoCapture> cap_;
    unsigned int err_count_;
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

        pipeline<std::shared_ptr<filter<opencv_frame_t> > > pipe;
        auto prod = std::make_shared<producer<opencv_frame_t> >(std::ref(*cam), is_stopped);
        pipe.add_filter(prod);

        auto circ = std::make_shared<hough_circles>();
        auto step1 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*circ), is_stopped);
        pipe.add_filter(step1);

        std::string svmpath = "tr2_80x80.svm";
        auto h = std::make_shared<hog>(svmpath);
        auto step2 = std::make_shared<transformer<opencv_frame_t> >(std::ref(*h), is_stopped);
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

int main(int argc, char** argv)
{
    app::context app_context;
    app_context.insert<app::args>(
        app::csbl::make_shared<app::args>(argc, argv));

    worker app(app_context);
    std::cout << "\nRunning on " << std::thread::hardware_concurrency()
              << " cores\n Press Ctrl+C to exit ..." << std::endl;

    return app::launch<app::common>(app, app_context);

    return 0;
}
