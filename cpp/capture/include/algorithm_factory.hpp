#ifndef ALGORITHM_FACTORY_HPP__
#define ALGORITHM_FACTORY_HPP__

#include <memory>

namespace boost
{
namespace program_options
{
    class variables_map;
}
}

class od_interface;

class algorithm_factory
{
public:
    static std::shared_ptr<od_interface> create_object_detector(const boost::program_options::variables_map&);
};

#endif // ALGORITHM_FACTORY_HPP
