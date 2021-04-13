cmake_minimum_required(VERSION 3.17)

project(buffer_pool 
    VERSION 0.1.0
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

enable_testing()

add_subdirectory(src)
add_subdirectory(test/unit)