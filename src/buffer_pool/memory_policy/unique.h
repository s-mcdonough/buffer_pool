#pragma once

#include <memory>

namespace buffer_pool
{
    template<class T, class Deleter>
    using Unique = std::unique_ptr<T, Deleter>;
}