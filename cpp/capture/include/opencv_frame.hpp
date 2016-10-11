#ifndef OPENCV_FRAME_HPP__
#define OPENCV_FRAME_HPP__

#include <memory>
#include <chrono>

namespace cv
{
class Mat;
} /* namespace cv */

struct data_t
{
    std::chrono::system_clock::time_point time_captured_;
    std::shared_ptr<cv::Mat> frame_;
};

#endif
