#include <catch2/catch.hpp>
#include <buffer_pool/pool.h>
#include <buffer_pool/memory_policy/shared.h>

#include <atomic>
#include <thread>

using namespace buffer_pool;

template<typename, class>
class FooPolicy {};

TEST_CASE("Uncomment to test pool traits", "[!hide]")
{
    // pool<int, FooPolicy> wont_compile;
}

TEMPLATE_TEST_CASE_SIG("Test constuction of buffer and retrieval", "[allocation][lifecycle]",
  ((typename T, template<typename, class> class Ptr), T, Ptr), (int,memory_policy::Unique), (int,memory_policy::Shared)) 
{
    pool<T, Ptr> bp;

    REQUIRE(bp.size() == 0);
    REQUIRE(bp.empty() == true);

    SECTION("Register an element with the pool")
    {
        const int value = 42;
        bp.manage(new int(value));

        // Size should be increased
        REQUIRE(bp.size() == 1);
        REQUIRE(bp.empty() == false);

        // Test getting the value
        {
            auto ptr_to_val = bp.get();
            CHECK(*ptr_to_val == value);

            // The buffer is removed from the pool
            REQUIRE(bp.size() == 0);
            REQUIRE(bp.empty() == true);
        }

        // Size should be increased
        REQUIRE(bp.size() == 1);
        REQUIRE(bp.empty() == false);
    }

    SECTION("Try re-registering an element with the pool")
    {
        const auto prev_nmanaged = bp.num_managed();
        auto* const pv = new int(42);
        CHECK(bp.manage(pv) == true);
        CHECK(bp.num_managed() == (prev_nmanaged + 1));

        // Try re-adding, this should fail
        CHECK(bp.manage(pv) == false);
        CHECK(bp.num_managed() == (prev_nmanaged + 1));
    }

    SECTION("In place construct an element with the pool")
    {
        const int approx_pi = 3;
        bp.emplace_manage(approx_pi);

        // Size should be increased
        REQUIRE(bp.size() == 1);
        REQUIRE(bp.empty() == false);

        // Test getting the value
        {
            auto ptr_to_val = bp.get();
            CHECK(*ptr_to_val == approx_pi);

            // The buffer is removed from the pool
            REQUIRE(bp.size() == 0);
            REQUIRE(bp.empty() == true);
        }

        // Size should be increased
        REQUIRE(bp.size() == 1);
        REQUIRE(bp.empty() == false);
    }
} 

TEMPLATE_TEST_CASE_SIG("queue properties query", "[properties]",
  ((typename T, template<typename, typename> class Ptr), T, Ptr), (double,memory_policy::Unique), (double,memory_policy::Shared))
{
    pool<T, Ptr> bp;
    const int num_items = 50;
    for(auto i=0; i<num_items; ++i)
    {
        bp.template emplace_manage<double>(3.14159);
        REQUIRE(bp.size() == i+1);
        REQUIRE(bp.capacity() >= i+1);
        REQUIRE(bp.empty() == false);
    }
}

TEMPLATE_TEST_CASE_SIG("a hostile user", "[lifecycle]",
  ((typename T, template<typename, typename> class Ptr), T, Ptr), 
  (int, memory_policy::Unique), (int, memory_policy::Shared)) 
{
    pool<T, Ptr> bp;
    bp.emplace_manage(1);
    bp.emplace_manage(2);
    REQUIRE(bp.size() == 2);

    // Only unique_ptr has the release method
    if constexpr (std::is_same_v<Ptr<T,std::default_delete<T>>, std::unique_ptr<T>>)
    {
        SECTION("releases the pointer")
        {
            {
                auto ptr = bp.get();
                REQUIRE(bp.size() == 1);
                auto* rp = ptr.release();
                delete rp;
            }
            // It is not inserted back into the pool
            CHECK(bp.size() == 1);
        }
    }

    SECTION("resets the pointer")
    {
        {
            auto ptr = bp.get();
            REQUIRE(bp.size() == 1);
            ptr.reset();
        }
        // It is inserted back into the pool
        CHECK(bp.size() == 2);
    }

    SECTION("swaps the pointer")
    {
        // TODO: Investigate. It does not seem possibe to swap the 
        // smart pointer as the deleter is private to the buffer_pool.
        // Investigate...
        // auto ptr = bp.get();
        // auto other = std::make_shared<T>(0);
        // std::swap(ptr, other);
    }
}

TEST_CASE("Blocking functionality", "[thread]")
{
    pool<int> bp{1,3,5,6};
    const auto sz = bp.size();
    std::atomic_int ctr(0);

    auto popper = [&]() -> void {
        auto p = bp.get();
        INFO("Number of elements" << bp.size());
    };

    std::thread prunner(popper);

    // CHECK()

    prunner.join();
}