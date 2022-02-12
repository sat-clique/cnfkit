cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

project(cnfkit VERSION 0.1.0)

get_directory_property(has_been_added_via_add_subdirectory PARENT_DIRECTORY)

if(has_been_added_via_add_subdirectory)
  add_library(cnfkit INTERFACE)
  target_include_directories(cnfkit INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
  # Building CNFKit in standalone mode, ie. it is not included via add_subdirectory()
  # in another project

  option(CNFKIT_ENABLE_TESTS "Enable testing" OFF)
  option(CNFKIT_TEST_ENABLE_SANITIZERS "Enable sanitizers for tests" OFF)
  option(CNFKIT_BUILD_DOCS "Build Doxygen documentation" OFF)

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CNFKIT_GNULIKE_COMPILER TRUE)
  endif()

  if (CNFKIT_TEST_ENABLE_SANITIZERS)
    if (NOT CNFKIT_GNULIKE_COMPILER)
      message(FATAL_ERROR "Setting sanitizer options is not supported for this compiler")
    endif()

    # Adding sanitizer flags globally to keep gtest and test compile flags the same
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  endif()

  find_package(ZLIB REQUIRED)
  find_package(LibArchive REQUIRED)

  add_library(cnfkit INTERFACE)
  target_include_directories(cnfkit INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  target_link_libraries(cnfkit INTERFACE ZLIB::ZLIB "${LibArchive_LIBRARIES}")
  target_include_directories(cnfkit INTERFACE "${LibArchive_INCLUDE_DIR}")

  install(DIRECTORY include/cnfkit DESTINATION include)
  install(TARGETS cnfkit EXPORT cnfkit INCLUDES DESTINATION include)
  install(EXPORT cnfkit DESTINATION lib/cmake/cnfkit FILE "cnfkitConfig.cmake")

  add_subdirectory(doc)
  add_subdirectory(testdeps)
  add_subdirectory(testsrc)
endif()

