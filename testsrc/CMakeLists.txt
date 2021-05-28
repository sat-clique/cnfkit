add_executable(cnfkit-tests
  ClauseTests.cpp
  DimacsParserTests.cpp
  DratParserTests.cpp
  LiteralTests.cpp
  TestUtils.cpp
  TestUtils.h
  TernaryTests.cpp
)

target_include_directories(cnfkit-tests PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(cnfkit-tests gtest gmock gmock_main ZLIB::ZLIB)

