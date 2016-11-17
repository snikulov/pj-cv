#ifndef OD_INTERFACE_HPP__
#define OD_INTERFACE_HPP__

#include "opencv_frame.hpp"

class od_interface
{
public:
    virtual bool has_objects(data_t&) = 0;
    virtual ~od_interface()
    {
    }
};

#endif // OD_INTERFACE_HPP__
