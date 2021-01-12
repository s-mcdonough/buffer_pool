#pragma once

#include <memory>

template<typename U, template<typename, typename> class Ptr>
class buffer_pool; // forward declare to make function declaration possible

template<class T, class Deleter>
class shared_ptr_adapter : public std::shared_ptr<T>
{
    friend class buffer_pool<T, shared_ptr_adapter>;

    T* release()
    {
        auto* ptr = this->get();
        this->template reset<T>(nullptr, [](T* p){});
        return ptr;
    }

    struct no_delete
    {
        void operator() (T* p) const {}
    };

public:
    shared_ptr_adapter(T* p)
        : std::shared_ptr<T>(p, no_delete())
    {}

    shared_ptr_adapter( std::shared_ptr<T>&& r ) noexcept
        : std::shared_ptr<T>(std::move(r))
    {}

    shared_ptr_adapter(T* p, Deleter d)
        : std::shared_ptr<T>(p, d)
    {}
};