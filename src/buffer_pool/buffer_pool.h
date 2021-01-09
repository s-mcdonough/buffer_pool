#pragma once

#include <deque>
#include <memory>

template<typename T>
class buffer_pool
{
    using bp_type = buffer_pool<T>;
    using internal_pointer_type = std::unique_ptr<T>;

    struct mover
    {
        mover() = delete;
        mover(bp_type* p_buffer_pool) {}

        void operator()(T* p) const
        {
            // We aren't deleting the object, just moving it
            _p_buffer_pool->return_to_buffer(p);
        }

        bp_type* _p_buffer_pool;
    };

    void return_to_buffer(T* object) { _queue.push_back(internal_pointer_type(object)); }

public:
    using value_type = T;
    using pointer_type = std::unique_ptr<T, mover>;
    using size_type = std::size_t;

    pointer_type get() { return pointer_type(_queue.pop_front().release()); }


    size_type capacity() const noexcept { return _queue.max_size(); }
    size_type size() const noexcept { return _queue.size(); }
    bool empty() const noexcept { return _queue.size() == 0; }

private:
    std::deque<internal_pointer_type> _queue;
};