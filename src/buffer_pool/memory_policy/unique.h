#pragma once

#include <memory>

namespace buffer_pool
{
    namespace memory_policy
    {
        template<typename T, class Deleter>
        using Unique = std::unique_ptr<T, Deleter>;
    }
}