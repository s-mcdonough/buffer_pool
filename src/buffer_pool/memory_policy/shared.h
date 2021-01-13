#pragma once

#include <memory>

namespace buffer_pool
{
    namespace memory_policy
    {
    namespace detail
    {

        /**
         * @brief Presents a unique_ptr like interface for shared_ptr.
         * This allows shared pointer to be used in a templated policy
         * based design. 
         * 
         * @tparam T element type
         * @tparam Deleter frees the underlying element (new to the adapter)
         */
        template<typename T, class Deleter>
        class shared_ptr_adapter : public std::shared_ptr<T>
        {
        public:
            
            shared_ptr_adapter( std::shared_ptr<T>&& r ) noexcept
                : std::shared_ptr<T>(std::move(r))
            {}

            shared_ptr_adapter(T* p, Deleter d)
                : std::shared_ptr<T>(p, d)
            {}

        }; // class shared_ptr_adapter
        
    } // namespace detail

    template<typename T, class Deleter>
    using Shared = detail::shared_ptr_adapter<T, Deleter>;

    } // namespace memory_policy
} // namespace buffer_pool