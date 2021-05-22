cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

project(CNFKit VERSION 0.1.0)

option(TEST_ENABLE_SANITIZERS "Enable address sanitizer for tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(zlib REQUIRED)

if (TEST_ENABLE_ASAN)
  add_compile_options(-fsanitize=address,undefined)
  add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(testdeps/googletest)
add_subdirectory(testsrc)

