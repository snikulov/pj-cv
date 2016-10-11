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

namespace app = boost::application;
namespace po  = boost::program_options;
namespace fs  = boost::filesystem;

class capture : private boost::noncopyable
{

public:
    capture(
        app::context& context, const std::string& url, monitor_queue<data_t>& q, const fs::path& p)
        : context_(context)
        , url_(url)
        , queue_(q)
        , img_path_(p)
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

        // Create output window for displaying frames.
        cv::namedWindow("Output");

        auto st   = context_.find<app::status>();
        int count = 0;

        std::vector<std::thread> tp;

        for (int i = 0; i < 3; ++i)
        {
            auto worker = std::make_shared<jpeg_writter>(context_, img_path_, queue_);
            tp.push_back(std::thread(*worker));
        }

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
        return 0;
    }

private:
    app::context& context_;
    std::string url_;
    monitor_queue<data_t>& queue_;
    fs::path img_path_;
};

int main(int argc, char* argv[])
{
    std::string url;
    std::string out_dir;

    po::options_description desc{ "Options" };
    // clang-format off
    desc.add_options()
        ("help,h", "Help screen")
        ("url,u", po::value<std::string>(&url)->default_value("rtsp://admin:admin@212.45.31.140:554/h264"),"camera url")
        ("out,o", po::value<std::string>(&out_dir)->default_value("images"), "output folder");
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

        app::context app_context;
        monitor_queue<data_t> frame_q;

        capture app(app_context, url, frame_q, outp);

        app_context.insert<app::args>(app::csbl::make_shared<app::args>(argc, argv));

        std::cout << "Running capture on " << url << "\nDetected "
                  << std::thread::hardware_concurrency() << " cores\n"
                  << "Press Ctrl+C to exit ..." << std::endl;

        return app::launch<app::common>(app, app_context);
    }
    std::cout << desc << "\n";
    return EXIT_SUCCESS;
}
