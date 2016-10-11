#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>
#include <boost/application.hpp>
#include <boost/program_options.hpp>

namespace app = boost::application;
namespace po = boost::program_options;

class capture
{

public:
    capture(app::context& context, const std::string& url)
        : context_(context)
        , url_(url)
    {
    }

    // param
    int operator()()
    {
        cv::VideoCapture vcap;
        cv::Mat image;

        //open the video stream and make sure it's opened
        if (!vcap.open(url_))
        {
            std::cout << "Error opening video stream, using " << url_ << std::endl;
            return -1;
        }

        //Create output window for displaying frames.
        cv::namedWindow("Output");

        auto st = context_.find<app::status>();

        int count = 0;
        while (st->state() != app::status::stopped)
        {
            if (!vcap.read(image))
            {
                std::cout << "No frame " << ++count << std::endl;
            }
            else
            {
                cv::imshow("Output", image);
            }
            // 1 milliseconds - allow opencv to process it own events
            (void)cv::waitKey(1);
        }
        return 0;
    }

private:
    app::context& context_;
    std::string url_;
};

int main(int argc, char* argv[])
{

    std::string url;
    po::options_description desc{ "Options" };
    desc.add_options()("help,h", "Help screen")("url,u", po::value<std::string>(&url)->default_value("rtsp://admin:admin@212.45.31.140:554/h264"), "IP camera url");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (!vm.count("help"))
    {
        url = vm["url"].as<std::string>();

        app::context app_context;
        capture app(app_context, url);

        app_context.insert<app::args>(
            app::csbl::make_shared<app::args>(argc, argv));

        std::cout << "Running capture on " << url
                  << "\nDetected " << std::thread::hardware_concurrency() << " cores\n"
                  << "Press Ctrl+C to exit ..." << std::endl;

        return app::launch<app::common>(app, app_context);
    }
    std::cout << desc << "\n";
    return EXIT_SUCCESS;
}
