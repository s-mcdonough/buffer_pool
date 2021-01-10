#include <catch2/catch.hpp>
#include <buffer_pool/buffer_pool.h>
#include <algorithm>

TEST_CASE("push/pop of an element", "[buffer_pool]")
{
    buffer_pool<int> bp;
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

TEST_CASE("size operators", "[buffer_pool]")
{
    buffer_pool<double> bp;
    const int num_items = 50;
    for(auto i=0; i<num_items; ++i)
    {
        bp.emplace_manage(3.14159);
        REQUIRE(bp.size() == i+1);
        REQUIRE(bp.capacity() >= i+1);
        REQUIRE(bp.empty() == false);
    }
}

TEST_CASE("hostile user", "[buffer_pool]")
{
    buffer_pool<int> bp;
    bp.emplace_manage(1);
    bp.emplace_manage(2);
    REQUIRE(bp.size() == 2);

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