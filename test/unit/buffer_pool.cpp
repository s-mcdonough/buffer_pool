#include <catch2/catch.hpp>
#include <buffer_pool/buffer_pool.h>
// #include <buffer_pool/shared_ptr_adapter.h>

using namespace buffer_pool;

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
        // TODO: Investigate, as I do not think it is possibe to swap the 
        // smart pointer as the deleter is private to the buffer_pool.
        // Investigate...
    }
}