cmake_minimum_required(VERSION 3.16)

project(anima)

set(BUILD_SHARED_LIBS False)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(
	GLOB_RECURSE 
	src 
	src/*.cpp
	external/imgui/src/*.cpp
)

add_executable(anima ${src})

add_subdirectory(external/glfw)
add_subdirectory(external/glew/build/cmake)
add_subdirectory(external/assimp)
add_subdirectory(external/spdlog)

target_include_directories(
	anima
 	PUBLIC
	external/glfw/include
	external/glew/include
	external/assimp/include
	external/spdlog/include
	external/imgui/src
)

target_link_libraries(
	anima
	glfw
	glew_s
	assimp
	spdlog
)