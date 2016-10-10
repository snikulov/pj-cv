#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

//using namespace cv;

int main(int, char**)
{
    cv::VideoCapture vcap;
    cv::Mat image;
    cv::Mat gray;

    const std::string vsH264 = "rtsp://admin:admin@212.45.31.140:554/h264";
    const std::string vsMJPEG = "rtsp://admin:admin@212.45.31.140:554/jpeg";
    /* it may be an address of an mjpeg stream, 
    e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

    //open the video stream and make sure it's opened
    if (!vcap.open(vsH264))
    {
        std::cout << "Error opening video stream H.264, Trying MJPEG..." << std::endl;
        if (!vcap.open(vsMJPEG))
        {
            std::cout << "Error opening video stream MJPEG... Exiting..." << std::endl;
            return -1;
        }
    }

    //Create output window for displaying frames.
    cv::namedWindow("Output Window", cv::WINDOW_KEEPRATIO | cv::WINDOW_NORMAL);

    std::vector<cv::Vec3f> circles;

    do
    {
        if (!vcap.read(image))
        {
            std::cout << "No frame" << std::endl;
        }
        else
        {

            // color to gray
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
 //           cv::GaussianBlur(gray, gray, {5, 5}, 1.5);
            cv::GaussianBlur(gray, gray, { 9, 9 }, 2, 2);
            circles.clear();
            cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 2, gray.rows/4, 200, 100);

            for (size_t i = 0; i < circles.size(); i++)
            {
                cv::Point center{ cvRound(circles[i][0]), cvRound(circles[i][1]) };
                int radius{ cvRound(circles[i][2]) };
                // draw the circle center
                circle(image, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
                // draw the circle outline
                circle(image, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
            }

            cv::imshow("Output Window", image);
        }
    } while (cv::waitKey(1) < 0);
}