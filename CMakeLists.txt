cmake_minimum_required(VERSION 3.17)

project(buffer_pool 
    VERSION 0.1.0
    LANGUAGES CXX)

include(FetchContent)

add_subdirectory(src)
add_subdirectory(test/unit)