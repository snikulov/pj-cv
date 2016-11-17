#include "pipe/pipeline.hpp"

bool pipeline::run()
{
    if (!is_running_)
    {
        is_running_ = true;
        if (!pstore_.empty())
        {
            for (auto it = pstore_.rbegin(); it != pstore_.rend(); ++it)
            {
                tgroup_.push_back(std::thread(*it));
            }
        }
    }
    else
    {
        is_running_ = false;
    }
    return is_running_;
}
