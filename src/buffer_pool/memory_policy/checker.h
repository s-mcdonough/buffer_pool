#pragma once

#include <memory>
#include <type_traits>

#include "buffer_pool/memory_policy/shared.h"
#include "buffer_pool/memory_policy/unique.h"

#define BUFFER_POOL_TEXTIFY(V) #V

#define BUFFER_POOL_MEMORY_POLICY_CHECK(T, P, TXT) \
    static_assert(std::disjunction_v< \
        std::is_same<P<T,std::default_delete<T>>, buffer_pool::memory_policy::Shared<T, std::default_delete<T>>>, \
        std::is_same<P<T,std::default_delete<T>>, buffer_pool::memory_policy::Unique<T, std::default_delete<T>>> \
        >, BUFFER_POOL_TEXTIFY(TXT));
