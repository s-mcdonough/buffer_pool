#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <type_traits>

#include "buffer_pool/shared_ptr_adapter.h"

template<typename T, template<typename, typename> class Ptr = std::unique_ptr>
class buffer_pool
{
    using internal_pointer_type = Ptr<T, std::default_delete<T>>;

    /**
     * @brief Moves the object back into the buffer pool once it goes out of scope.
     * 
     * A custom deleter for the wrapped buffer object.
     */
    struct mover
    {
        mover() = delete;
        mover(buffer_pool* p_buffer_pool, T* ptr) 
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

        buffer_pool* _p_buffer_pool;
        T* _ptr;
    };

public:
    using value_type = T;
    using pointer_type = Ptr<T, mover>;
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
        // TODO: Currently, there is UB if size() == 0. 
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

    template<class U, class... Args>
    typename std::enable_if_t<std::is_same_v<internal_pointer_type, 
        std::unique_ptr<U>>, void>
    emplace_manage(Args&&... args)
    {
        std::lock_guard lk(_mutex);
        _queue.emplace_back(std::make_unique<U>(std::forward<Args>(args)...));
    }

    template<class U, class... Args>
    typename std::enable_if_t<
                 std::is_same_v<internal_pointer_type, 
                                shared_ptr_adapter<U, std::default_delete<U>>>,
                 void>
    emplace_manage(Args&&... args)
    {
        std::lock_guard lk(_mutex);
        _queue.emplace_back(std::make_shared<U>(std::forward<Args>(args)...));
    }

    size_type capacity() const noexcept { return _queue.max_size(); }
    size_type size() const noexcept { return _queue.size(); }
    bool empty() const noexcept { return _queue.size() == 0; }

private:
    std::deque<internal_pointer_type> _queue;
    std::mutex _mutex;
};