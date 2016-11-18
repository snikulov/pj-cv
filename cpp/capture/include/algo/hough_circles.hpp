#ifndef HOUGH_CIRCLES_HPP__
#define HOUGH_CIRCLES_HPP__

#include "algo/od_interface.hpp"
#include <opencv2/opencv.hpp>

class hough_circles : public od_interface
{
public:
    bool has_objects(opencv_frame_t& d)
    {
        cv::Mat gray;
        std::vector<cv::Vec3f> circles;

        cv::cvtColor(*(d.frame_), gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, gray, { 5, 5 }, 1.5, 1.5);
        cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 2, 300, 50, 300, 350, 650);

        if (!circles.empty())
        {
            d.circles_.reset(new std::vector<cv::Vec3f>());
            std::swap(*(d.circles_), circles);
            return true;
        }
        return false;
    }

    opencv_frame_t operator()(opencv_frame_t d)
    {
        opencv_frame_t tmp = d;
        if (tmp.frame_ && !tmp.frame_->empty())
        {
            (void)has_objects(tmp);
        }
        return tmp;
    }
};

#endif // HOUGH_CIRCLES_HPP__
