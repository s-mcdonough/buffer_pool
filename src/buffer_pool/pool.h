#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <type_traits>

#include "buffer_pool/memory_policy/shared.h"
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
                    delete p;
                }
                
            }
        }

    private:
        pool* _p_buffer_pool;
        T* _ptr;
    };

    void return_to_pool(T* object)
    {
        std::unique_lock lk(_mutex);
        _queue.push_back(internal_pointer_type(object));
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

    pool(std::initializer_list<internal_pointer_type> init)
        : _queue(init)
        , _mutex()
        , _total_managed(init.size())
    {}

    ~pool() = default;
    pool(const pool&) = delete;
    pool& operator= (const pool&) = delete;

    /**
     * @brief Gets an buffer from the pool.
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
     * @brief Bring an object under management of the pool
     * 
     * @param object 
     */
    void manage(T* object) 
    { 
        return_to_pool(object); 
        // TODO: Check to make sure the object isnt already managed
        ++_total_managed;
    }

    template<class... Args>
    void emplace_manage(Args&&... args)
    {
        std::lock_guard lk(_mutex);
        _queue.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        ++_total_managed;
    }

    [[nodiscard]] size_type capacity() const noexcept { return _queue.max_size(); }
    [[nodiscard]] size_type num_managed_buffers() const noexcept { return _total_managed; }
    [[nodiscard]] size_type size() const noexcept { return _queue.size(); }
    [[nodiscard]] bool empty() const noexcept { return _queue.size() == 0; }

private:
    std::condition_variable _cv;
    std::deque<internal_pointer_type> _queue;
    std::mutex _mutex;
    std::atomic_size_t _total_managed;
};

} // namespace buffer_pool