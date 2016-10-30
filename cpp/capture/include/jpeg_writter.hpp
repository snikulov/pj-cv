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
#include "algo/od_interface.hpp"

class jpeg_writter
{
public:
    jpeg_writter(boost::application::context& ctx, const boost::filesystem::path& p,
        monitor_queue<data_t>& q, std::shared_ptr<od_interface> det)
        : context_(ctx)
        , path_(p)
        , queue_(q)
        , detector_(det)
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
                if (detector_->has_objects(data))
                {
                    if (write_image(data))
                    {
                        std::cerr << "Writted image: " << get_fname(data) << std::endl;
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


    std::string get_fname(const data_t& d)
    {
        using namespace date;
        std::ostringstream ss;
        ss << d.time_captured_ << "-" << std::this_thread::get_id();
        std::string ret{ ss.str() };

        auto pred = [](char& ch) { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
        std::replace_if(ret.begin(), ret.end(), pred, '-');
        ret += ".jpg";
        return ret;
    }

    boost::application::context& context_;
    boost::filesystem::path path_;
    monitor_queue<data_t>& queue_;
    std::shared_ptr<od_interface> detector_;
};

#endif
