#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>
#include <buffer_pool/pool.h>
#include <buffer_pool/memory_policy/shared.h>

#include <atomic>
#include <thread>

#include <type_traits>

using namespace buffer_pool;

TEST_CASE("Benchmarks", "[benchmark]")
{
    pool<int, buffer_pool::memory_policy::Shared> bp;
    bp.emplace_manage(1);
    bp.emplace_manage(2);
    REQUIRE(bp.size() == 2);

    int* ptr = new int(1);

    BENCHMARK("Manage")
    {
        return bp.manage(ptr);
    };

    BENCHMARK("Getter")
    {
        return bp.get();
    };
}