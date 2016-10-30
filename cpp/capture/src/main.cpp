#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>
#include <boost/application.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include "opencv_frame.hpp"
#include "monitor_queue.hpp"
#include "jpeg_writter.hpp"

#include "algo/od_interface.hpp"
#include "algo/hough_circles.hpp"
#include "algo/hog_dlib.hpp"

namespace app = boost::application;
namespace po  = boost::program_options;
namespace fs  = boost::filesystem;

class capture : private boost::noncopyable
{

public:
    capture(
        app::context& context, const std::string& url, monitor_queue<data_t>& q, const fs::path& p,
        const std::string& algo, const std::string& svm_path)
        : context_(context)
        , url_(url)
        , queue_(q)
        , img_path_(p)
        , algo_(algo)
        , svm_path_(svm_path)
    {
    }

    // param
    int operator()()
    {
        cv::VideoCapture vcap;
        data_t frame;

        // open the video stream and make sure it's opened
        if (!vcap.open(url_))
        {
            std::cout << "Error opening video stream, using " << url_ << std::endl;
            return -1;
        }

        std::shared_ptr<od_interface> detector;

        if (algo_ == "circles")
        {
            detector.reset(new hough_circles);
        }
        else if (algo_ == "hog")
        {
            fs::path p{svm_path_};
            if (fs::exists(p) && fs::is_regular_file(p))
            {
                detector.reset(new hog(svm_path_));
            }
            else
            {
                std::cout << "File not found: " << fs::absolute(p).string() << std::endl;
                return -1;
            }
        }
        else
        {
            std::cout << "Unknown algo " << algo_ << " Supported circles or hog" << std::endl;
            return -1;
        }

        // Create output window for displaying frames.
        cv::namedWindow("Output");

        auto st   = context_.find<app::status>();
        int count = 0;

        auto processor = std::make_shared<jpeg_writter>(context_, img_path_, queue_, detector);
        std::thread wt{ *processor };

        while (st->state() != app::status::stopped)
        {
            frame.frame_ = std::make_shared<cv::Mat>();

            if (!vcap.read(*(frame.frame_)))
            {
                std::cout << "No frame " << ++count << std::endl;
                if (count > 5)
                {
                    if (vcap.open(url_))
                    {
                        count = 0;
                    }
                    else
                    {
                        std::cout << "Unable to re-open stream. Exiting..." << std::endl;
                        st->state(app::status::stopped);
                        return -1;
                    }
                }
            }
            else
            {
                count                = 0;
                frame.time_captured_ = std::chrono::system_clock::now();
                queue_.enqueue(frame);
                cv::imshow("Output", *(frame.frame_));
            }
            // 1 milliseconds - allow opencv to process it own events
            (void)cv::waitKey(1);
        }

        wt.join();
        return 0;
    }

private:
    app::context& context_;
    std::string url_;
    monitor_queue<data_t>& queue_;
    fs::path img_path_;
    std::string algo_;
    std::string svm_path_;
};

int main(int argc, char* argv[])
{
    std::string url;
    std::string out_dir;
    std::string algo;

    po::options_description desc{ "Options" };
    // clang-format off
    desc.add_options()
        ("help,h", "Help screen")
        ("url,u", po::value<std::string>(&url)->default_value("rtsp://admin:admin@212.45.31.140:554/h264"), "camera url")
        ("out,o", po::value<std::string>(&out_dir)->default_value("images"), "output folder")
        ("algo, a", po::value<std::string>(&out_dir)->default_value("circles"), "detection algorithm[circles|hog]")
        ("svmpath, s", po::value<std::string>(&out_dir)->default_value("object_detector.svm"), "path to obj detector svm")
        ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (!vm.count("help"))
    {
        url = vm["url"].as<std::string>();
        fs::path outp{ vm["out"].as<std::string>() };
        if (!fs::exists(outp))
        {
            fs::create_directory(outp);
        }

        algo = vm["algo"].as<std::string>();
        auto svm_path = vm["svmpath"].as<std::string>();

        app::context app_context;
        monitor_queue<data_t> frame_q;

        capture app(app_context, url, frame_q, outp, algo, svm_path);

        app_context.insert<app::args>(app::csbl::make_shared<app::args>(argc, argv));

        std::cout << "Running capture on " << url << "\nDetected "
                  << std::thread::hardware_concurrency() << " cores\n"
                  << "Press Ctrl+C to exit ..." << std::endl;

        return app::launch<app::common>(app, app_context);
    }
    std::cout << desc << "\n";
    return EXIT_SUCCESS;
}
