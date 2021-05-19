add_executable(cnfkit-tests
  DimacsTests.cpp
  LiteralTests.cpp
  TernaryTests.cpp
)

target_include_directories(cnfkit-tests PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(cnfkit-tests gtest gmock gmock_main ZLIB::ZLIB)

