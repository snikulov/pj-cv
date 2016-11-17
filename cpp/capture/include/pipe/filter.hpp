#ifndef FILTER_HPP__
#define FILTER_HPP__

class filter
{
public:
    filter()
    {
    }

    /// thread function
    virtual void operator()() = 0;

    /// as usual
    virtual ~filter()
    {
    }
};

#endif // PIPE_ELEMENT_HPP__
