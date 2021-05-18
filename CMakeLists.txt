cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

project(CNFKit VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(testdeps/googletest)
add_subdirectory(testsrc)
