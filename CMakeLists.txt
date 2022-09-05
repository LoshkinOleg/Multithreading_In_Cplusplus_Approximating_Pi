cmake_minimum_required (VERSION 3.14) # Define minimal CMake version.
project(Multithreading) # Name the visual studio solution.

# Set the C++20 as the C++ standard to use (ISO/IEC 14882).
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure build dir exists. We'll be relying on this for finding relative paths in the code and any external tools.
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/build/")
	message(FATAL_ERROR "Please specify an out-of-source directory 'build/' in the project's root directory. If you don't know what an out-of-source build is, here's a link: https://cmake.org/cmake/help/book/mastering-cmake/chapter/Getting%20Started.html")
endif()

file(GLOB_RECURSE app_include Application/include/*.h)
file(GLOB_RECURSE app_src Application/src/*.cpp)
add_executable(Application ${app_include} ${app_src})
target_include_directories(Application PRIVATE ${app_include})