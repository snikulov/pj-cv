#ifndef PIPELINE_HPP__
#define PIPELINE_HPP__

#include <list>
#include <memory>
#include <thread>

class filter;

class pipeline
{
public:
    pipeline();

    ~pipeline();

    void add_filter(filter& f);

    bool run();

    /// no copy
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

private:
    std::list<std::shared_ptr<filter> > pstore_;
    bool is_running_;
    std::list<std::thread> tgroup_;
};

#endif // PIPELINE_HPP__
