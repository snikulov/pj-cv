////////////////////////////////////////////////////////////////////////////////////////////////
///	Copyright (C) 2016-2017, Sergei Nikulov (sergey.nikulov@gmail.com)
///	All rights reserved.
///
///	Redistribution and use in source and binary forms, with or without
///	modification, are permitted provided that the following conditions are met:
///	* Redistributions of source code must retain the above copyright
///	notice, this list of conditions and the following disclaimer.
///	* Redistributions in binary form must reproduce the above copyright
///	notice, this list of conditions and the following disclaimer in the
///	documentation and/or other materials provided with the distribution.
///	* Neither the name of the <organization> nor the
///	names of its contributors may be used to endorse or promote products
///	derived from this software without specific prior written permission.
///
///	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
///	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
///	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
///	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
///	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
///	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
///	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////


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
