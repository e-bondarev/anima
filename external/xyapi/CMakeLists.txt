cmake_minimum_required(VERSION 3.16)

project(xyapi)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE src src/*.cpp)

add_library(xyapi ${src})

target_include_directories(xyapi PUBLIC include/xyapi)