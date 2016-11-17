#ifndef OPENCV_FRAME_HPP__
#define OPENCV_FRAME_HPP__

#include <memory>
#include <chrono>

namespace cv
{
class Mat;
} // namespace cv

struct data_t
{
    /// time, when frame captured
    std::chrono::system_clock::time_point time_captured_;

    /// frame with image data
    std::shared_ptr<cv::Mat> frame_;
};

#endif
