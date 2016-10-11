#ifndef JPEG_WRITTER_HPP__
#define JPEG_WRITTER_HPP__

#include <sstream>
#include <date.h>

#include <opencv2/opencv.hpp>

#include <boost/application.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include "opencv_frame.hpp"
#include "monitor_queue.hpp"

class jpeg_writter
{
public:
    jpeg_writter(boost::application::context& ctx, const boost::filesystem::path& p,
        monitor_queue<data_t>& q)
        : context_(ctx)
        , path_(p)
        , queue_(q)
    {
    }

    void operator()()
    {
        auto st = context_.find<boost::application::status>();

        data_t data;

        while (st->state() != boost::application::status::stopped)
        {
            data = queue_.dequeue();
            if (data.frame_)
            {
                if (is_circles(data))
                {
                    if (write_image(data))
                    {
                        std::cerr << "error writting image " << get_fname(data) << std::endl;
                    }
                }
            }
        }
    }

private:
    jpeg_writter() = delete;

    bool write_image(const data_t& d)
    {
        boost::filesystem::path f{ path_ };
        f /= get_fname(d);
        return cv::imwrite(f.string(), *(d.frame_));
    }

    bool is_circles(const data_t& d)
    {
        cv::Mat gray;
        std::vector<cv::Vec3f> circles;

        cv::cvtColor(*(d.frame_), gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, gray, { 9, 9 }, 2, 2);
        cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 2, gray.rows / 4, 200, 100);

        return !circles.empty();
    }

    std::string get_fname(const data_t& d)
    {
        using namespace date;
        std::ostringstream ss;
        ss << std::this_thread::get_id() << "-" << d.time_captured_;
        std::string ret{ ss.str() };

        auto pred = [](char& ch) { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
        std::replace_if(ret.begin(), ret.end(), pred, '-');
        ret += ".jpg";
        return ret;
    }

    boost::application::context& context_;
    boost::filesystem::path path_;
    monitor_queue<data_t>& queue_;
};

#endif
