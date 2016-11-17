#ifndef OPENCV_FRAME_HPP__
#define OPENCV_FRAME_HPP__

#include <memory>
#include <chrono>
#include <opencv2/opencv.hpp>

struct data_t
{
    /// time, when frame captured
    std::chrono::system_clock::time_point time_captured_;

    /// frame with image data
    std::shared_ptr<cv::Mat> frame_;

    /// opencv Circles found
    std::shared_ptr<std::vector<cv::Vec3f> > circles_;
    std::shared_ptr<std::vector<cv::Rect> > squares_;
};

#endif
