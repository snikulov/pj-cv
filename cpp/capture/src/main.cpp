#include <iostream>
#include <opencv2/opencv.hpp>

//using namespace cv;

int main(int, char**)
{
    cv::VideoCapture vcap;
    cv::Mat image;

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

    do
    {
        if (!vcap.read(image))
        {
            std::cout << "No frame" << std::endl;
        }
        else
        {
            cv::imshow("Output Window", image);
        }
    } while (cv::waitKey(1) < 0);
}