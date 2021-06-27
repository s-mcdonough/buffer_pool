#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>
#include <buffer_pool/pool.h>
#include <buffer_pool/memory_policy/shared.h>

#include <atomic>
#include <thread>
#include <iostream>

#include <type_traits>

using namespace buffer_pool;

template<typename T>
struct smol
{
    smol() = delete;
    smol(T value) : t(value) {}
    ~smol() = default;
    T t;
};

template<typename T>
struct Huge
{
    Huge() = delete;
    Huge(T f) : a{} { a.fill(f); }
    ~Huge() = default;
    std::array<T, 8192> a;
};
// struct Expensive
// {

// };

// Reduce template syntax noise
using value_type = int;
using smol_v = smol<value_type>;
using Huge_v = Huge<value_type>;

// TEST_CASE("Benchmarks", "[benchmark]")
TEMPLATE_TEST_CASE_SIG("Benchmarks", "[benchmark]",
  ((typename T, template<typename, typename> class Ptr), T, Ptr), 
  (smol_v, memory_policy::Unique), (smol_v, memory_policy::Shared),
  (Huge_v, memory_policy::Unique), (Huge_v, memory_policy::Shared)) 
{
    pool<T, Ptr> bp;
    bp.emplace_manage(1);
    bp.emplace_manage(2);
    REQUIRE(bp.size() == 2);

    T* ptr = new T(1);
    auto up = std::make_unique<T>(1);

    BENCHMARK("Make Unique (baseline)")
    {
        return std::make_unique<T>(3141);
    };

    BENCHMARK("Raw Manage")
    {
        return bp.manage(ptr);
    };

    BENCHMARK("Unique-Ptr Manage")
    {
        return bp.manage(std::move(up));
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