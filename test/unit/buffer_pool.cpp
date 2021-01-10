#include <catch2/catch.hpp>
#include <buffer_pool/buffer_pool.h>

TEST_CASE("size operators", "[buffer_pool]")
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