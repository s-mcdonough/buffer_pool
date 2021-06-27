#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>
#include <buffer_pool/pool.h>
#include <buffer_pool/memory_policy/shared.h>

#include <atomic>
#include <thread>

#include <type_traits>

using namespace buffer_pool;

// TEST_CASE("Benchmarks", "[benchmark]")
TEMPLATE_TEST_CASE_SIG("Benchmarks", "[benchmark]",
  ((typename T, template<typename, typename> class Ptr), T, Ptr), 
  (int, memory_policy::Unique), (int, memory_policy::Shared)) 
{
    pool<T, Ptr> bp;
    bp.emplace_manage(1);
    bp.emplace_manage(2);
    REQUIRE(bp.size() == 2);

    T* ptr = new T(1);

    BENCHMARK("Manage")
    {
        return bp.manage(ptr);
    };

    BENCHMARK("Getter")
    {
        return bp.get();
    };

    BENCHMARK("Try-getter")
    {
        return bp.try_get();
    };
}