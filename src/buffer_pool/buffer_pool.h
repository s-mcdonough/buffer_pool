#pragma once

#include <deque>
#include <memory>
#include <mutex>

/**
 * @brief A 
 * 
 * @tparam T underlying type to manage
 */
template<typename T>
class buffer_pool
{
    using bp_type = buffer_pool<T>;
    using internal_pointer_type = std::unique_ptr<T>;

    /**
     * @brief Moves the object back into the buffer pool once it goes out of scope.
     * 
     * A custom deleter for the wrapped buffer object.
     */
    struct mover
    {
        mover() = delete;
        mover(bp_type* p_buffer_pool, T* ptr) 
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
                    _p_buffer_pool->manage(p);
                }
                else
                {
                    // Otherwise, free the pointer
                    delete p;
                }
                
            }
        }

        bp_type* _p_buffer_pool;
        T* _ptr;
    };

public:
    using value_type = T;
    using pointer_type = std::unique_ptr<T, mover>;
    using size_type = std::size_t;

    buffer_pool() = default;
    ~buffer_pool() = default;
    buffer_pool(const buffer_pool&) = delete;
    buffer_pool& operator= (const buffer_pool&) = delete;

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
        std::lock_guard lk(_mutex);
        auto* raw_ptr = _queue.front().release();
        auto ptr = pointer_type(raw_ptr, mover(this, raw_ptr));
        _queue.pop_front();
        return ptr;
    }

    /**
     * @brief 
     * 
     * @param object 
     */
    void manage(T* object) 
    { 
        std::lock_guard lk(_mutex);
        _queue.push_back(internal_pointer_type(object)); 
    }

    template<class... Args>
    void emplace_manage(Args&&... args)
    {
        std::lock_guard lk(_mutex);
        _queue.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    size_type capacity() const noexcept { return _queue.max_size(); }
    size_type size() const noexcept { return _queue.size(); }
    bool empty() const noexcept { return _queue.size() == 0; }

private:
    std::deque<internal_pointer_type> _queue;
    std::mutex _mutex;
};