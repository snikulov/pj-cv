#ifndef PIPELINE_HPP__
#define PIPELINE_HPP__

#include <list>
#include <memory>
#include <thread>

#include "producer.hpp"

template <class T, class D>
class pipeline
{
public:
    pipeline()
        : is_running_(false)
    {
    }

    ~pipeline()
    {
        if (is_running_)
        {
            for (auto& elm : tgroup_)
            {
                elm->join();
            }
        }
        qlist_.clear();
    }

    void add_filter(T f)
    {
        if (!filters_.empty())
        {
            auto glue = std::make_shared<monitor_queue<D> >();
            T& prev_step = filters_.back();
            prev_step->output_ = glue;
            f->input_ = glue;

            // for debug
            qlist_.push_back(glue);
        }
        filters_.push_back(f);
    }

    bool run()
    {
        if ((!is_running_) && (!filters_.empty()))
        {
            is_running_ = true;
            for (auto it = filters_.rbegin(); it != filters_.rend(); ++it)
            {

                auto elm = *it;
                tgroup_.push_back(std::make_shared<std::thread>(std::ref(*elm)));
#if 0
                // some magic
                auto nptr = elm.get();
                auto prodptr = dynamic_cast<producer<T>*>(nptr);
                if (prodptr)
                {
                    tgroup_.push_back(std::make_shared<std::thread>(std::ref(*prodptr)));
                }
                else
                {
                    auto transptr = dynamic_cast<transformer<T>*>(nptr);
                    if (transptr)
                    {
                        tgroup_.push_back(std::make_shared<std::thread>(std::ref(*transptr)));
                    }
                    else
                    {
                        auto finptr = dynamic_cast<sink<T>*>(nptr);
                        if (finptr)
                        {
                            tgroup_.push_back(std::make_shared<std::thread>(std::ref(*finptr)));
                        }
                        else
                        {
                            throw std::runtime_error("Unable cast to filter!");
                        }
                    }
                }
#endif
            }
        }
        return is_running_;
    }

    /// no copy
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

private:
    std::list<T> filters_;
    bool is_running_;
    std::list<std::shared_ptr<std::thread> > tgroup_;

    // for debug
    std::list<std::shared_ptr<monitor_queue<D> > > qlist_;
};

#endif // PIPELINE_HPP__
