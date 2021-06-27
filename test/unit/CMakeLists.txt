# Download Catch2 using FetchContent
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.6)

# This line configures/builds the item specifed in _Declare and is done at
# configuration time. Targets in the Catch2 namespace are now avaible for use.
FetchContent_MakeAvailable(catch2)

# A slightly heavy handed approach to add a specific script into our cmake
# project. Used for catch_discover_tests()
include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)

#
# Setup Tests
#

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(ut_buffer_pool)

target_sources(ut_buffer_pool PRIVATE main.cpp benchmarks.cpp buffer_pool.cpp)
target_link_libraries(ut_buffer_pool PRIVATE buffer_pool Catch2::Catch2 Threads::Threads)

# Links catch tests into CTest
catch_discover_tests(ut_buffer_pool)