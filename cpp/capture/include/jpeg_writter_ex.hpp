#ifndef JPEG_WRITTER_EX_HPP
#define JPEG_WRITTER_EX_HPP
#include <sstream>
#include <date.h>

#include <opencv2/opencv.hpp>

#include <boost/application.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include "opencv_frame.hpp"

class jpeg_writter_ex
{
public:
    jpeg_writter_ex(const std::string& p)
        : path_(p)
    {
    }

    void operator()(opencv_frame_t d)
    {
        if (d.frame_ && !d.frame_->empty())
        {
            write_image(d);
        }
    }

private:
    bool write_image(const opencv_frame_t& d)
    {
        boost::filesystem::path f{ path_ };
        f /= get_fname(d);
        return cv::imwrite(f.string(), *(d.frame_));
    }

    std::string get_fname(const opencv_frame_t& d)
    {
        using namespace date;
        std::ostringstream ss;
        ss << d.time_captured_ << "-" << std::this_thread::get_id();
        std::string ret{ ss.str() };

        auto pred = [](char& ch)
        { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
        std::replace_if(ret.begin(), ret.end(), pred, '-');
        ret += ".jpg";
        return ret;
    }

    boost::filesystem::path path_;
};

#endif // JPEG_WRITTER_EX_HPP
