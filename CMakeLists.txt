cmake_minimum_required(VERSION 3.16)

project(anima)

file(GLOB_RECURSE src src/*.cpp)

add_executable(anima ${src})