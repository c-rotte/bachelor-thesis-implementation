cmake_minimum_required(VERSION 3.18)
project(b-epsilon)

set(CMAKE_CXX_STANDARD 20)

# enable compiler warnings
string(APPEND CMAKE_CXX_FLAGS "-O0 -Wall -Wextra")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Infrastructure)

add_subdirectory(src)
add_subdirectory(test)