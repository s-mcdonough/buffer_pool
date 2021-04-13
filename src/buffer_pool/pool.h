// Copyright (c) 2021 Sean McDonough
//
// This file is part of BufferPool.
//
// BufferPool is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BufferPool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BufferPool.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <type_traits>

#include "buffer_pool/memory_policy/checker.h"
#include "buffer_pool/memory_policy/unique.h"

namespace buffer_pool
{

/**
 * @brief 
 * 
 * @tparam T underlying type to manage
 * @tparam MemoryPolicy 
 */
template<typename T, template<typename, class> class MemoryPolicy = memory_policy::Unique>
class pool
{
    BUFFER_POOL_MEMORY_POLICY_CHECK(T, MemoryPolicy, "Invalid pool memory policy");
    static_assert((!std::is_polymorphic_v<T> || std::has_virtual_destructor_v<T>) && 
                  std::is_destructible_v<T>
                  ,"T must be safely destructable!");

    using internal_pointer_type = std::unique_ptr<T>;

    /**
     * @brief Moves the object back into the buffer pool once it goes out of scope.
     * 
     * A custom deleter for the wrapped buffer object.
     */
    struct mover
    {
        mover() = delete;
        mover(pool* p_buffer_pool, T* ptr) 
            : _p_buffer_pool(p_buffer_pool)
            , _ptr(ptr) {}

        void operator()(T* p) const
        {
            // Handle the case where the underlying pointer has been released
            if (p) 
            {
                // Handle the case where the initial pointer has been replaced
                if (p == _ptr) 
                {
                    // If it matches, we aren't deleting the object, but putting it back in the queue
                    _p_buffer_pool->return_to_pool(p);
                }
                else
                {
                    // Otherwise, free the pointer
                    delete p; // TODO: Handle the case where T has been allocated with new[], via template specalization (see unique_ptr for more detail).
                }
                
            }
        }

    private:
        pool* const _p_buffer_pool;
        const T* const _ptr;
    };

    void return_to_pool(T* object)
    {
        {
            std::lock_guard lk(_mutex);
            _queue.push_back(internal_pointer_type(object));
        } // unlock the mutex
        _cv.notify_one();
    }

public:
    using element_type = T;
    using pointer_type = MemoryPolicy<T, mover>;
    using size_type = std::size_t;

    pool() 
        : _queue()
        , _mutex()
        , _total_managed(0)
    {}

    pool(std::initializer_list<T> init)
        : _queue()
        , _mutex()
        , _total_managed(0)
    {
        for (auto& i : init)
        {
            emplace_manage(i);
        }
    }

    pool(std::initializer_list<internal_pointer_type> init)
        : _queue(init)
        , _mutex()
        , _total_managed(init.size())
    {}

    ~pool() = default;
    pool(const pool&) = delete;
    pool& operator= (const pool&) = delete;

    /**
     * @brief Gets a buffer from the pool. Blocking.
     * 
     * This decreases the number of available objets gettable from the pool. 
     * 
     * @return pointer_type A buffer for use. Returned to the pool once 
     *                      the buffer goes out of scope.
     */
    pointer_type get() 
    { 
        std::unique_lock lk(_mutex);

        // Prevent UB where poping off an empty queue
        if (size() == 0)
        {
            _cv.wait(lk, [&](){ return this->size() > 0; });
        } 
        
        auto* raw_ptr = _queue.front().release();
        _queue.pop_front();
        auto ptr = pointer_type(raw_ptr, mover(this, raw_ptr));
        return ptr;
    }

    /**
     * @brief Attempts to get a buffer from the pool, returns 
     * nullptr if there is none. Non-blocking.
     * 
     * @return pointer_type 
     */
    pointer_type try_get() 
    {
        {
            std::lock_guard lk(_mutex);
            if (_queue.size() > 0)
            {
                auto* raw_ptr = _queue.front().release();
                _queue.pop_front();
                auto ptr = pointer_type(raw_ptr, mover(this, raw_ptr));
                return ptr;
            }
        }
        
        return pointer_type(nullptr, mover(this, nullptr));
    }

    /**
     * @brief Bring an object under management of the pool.
     * 
     * @param object Pointer to the object to manage.
     * @return False if the object is already managed by the pool, true otherwise.
     */
    bool manage(T* object) 
    {
        const bool unique = std::none_of(_queue.begin(), _queue.end(), [object](const internal_pointer_type& p)->bool
            { 
                return p.get() == object;
            });

        if (unique)
        {
            return_to_pool(object);
            ++_total_managed;
        }

        return unique;        
    }

    template<class... Args>
    void emplace_manage(Args&&... args)
    {
        {
            std::unique_lock lk(_mutex);
            _queue.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
            ++_total_managed;
        } // unlocks the mutex
        _cv.notify_one();
    }

    /**
     * @brief Capacity of the underlying container
     * 
     * @return size_type 
     */
    size_type capacity() const noexcept { return _queue.max_size(); }
    size_type num_managed() const noexcept { return _total_managed; }
    size_type size() const noexcept { return _queue.size(); }
    [[nodiscard]] bool empty() const noexcept { return _queue.size() == 0; }

private:
    std::condition_variable _cv;
    std::deque<internal_pointer_type> _queue;
    std::mutex _mutex;
    std::atomic_size_t _total_managed;
};

} // namespace buffer_pool