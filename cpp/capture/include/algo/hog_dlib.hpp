#ifndef HOG_DLIB_HPP__
#define HOG_DLIB_HPP__


#include "algo/od_interface.hpp"

#include <dlib/opencv/cv_image.h> 
#include <dlib/svm_threaded.h>
#include <dlib/image_processing.h>
#include <dlib/data_io.h>
#include <dlib/gui_widgets.h>
#include <date.h>

using img_scanner_t = dlib::scan_fhog_pyramid<dlib::pyramid_down<6>>;

class hog : public od_interface
{
public:
    explicit hog(const std::string& path_to_svm)
        : detector_(new dlib::object_detector<img_scanner_t>())
        , svm_path_(path_to_svm)
    {
        using namespace date;
        std::cout << std::chrono::system_clock::now() << " loading SVM from " << svm_path_ << std::endl;
        dlib::deserialize(svm_path_) >> *detector_;
        std::cout << std::chrono::system_clock::now() << " finished loading" << std::endl;
    }

    bool has_objects(data_t& d)
    {
        cv::Mat tmp = *d.frame_;
        dlib::cv_image<dlib::bgr_pixel> cimg(tmp);
        std::vector<dlib::rectangle> dets = (*detector_)(cimg);
        return !dets.empty();
    }

private:
    std::unique_ptr<dlib::object_detector<img_scanner_t> > detector_;
    std::string svm_path_;
};

#endif  // HOG_DLIB_HPP__
                                    