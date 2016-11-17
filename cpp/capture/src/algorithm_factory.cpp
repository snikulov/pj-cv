#include "algorithm_factory.hpp"
#include "algo/od_interface.hpp"
#include "algo/hog_dlib.hpp"
#include "algo/hough_circles.hpp"

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

std::shared_ptr<od_interface> algorithm_factory::create_object_detector(const boost::program_options::variables_map& vm)
{
    std::shared_ptr<od_interface> ret_val;

    auto algo = vm["algo"].as<std::string>();

    if (boost::iequals(algo, "circles"))
    {
        ret_val.reset(new hough_circles);
    }
    else if (boost::iequals(algo, "hog"))
    {
        namespace fs = boost::filesystem;

        fs::path p{ vm["svmpath"].as<std::string>() };
        if (fs::exists(p) && fs::is_regular_file(p))
        {
            ret_val.reset(new hog(p.string()));
        }
        else
        {
            std::cout << "File not found: " << fs::absolute(p).string()
                      << std::endl;
        }
    }
    else
    {
        // log unknown algorithm
    }
    return ret_val;
}
