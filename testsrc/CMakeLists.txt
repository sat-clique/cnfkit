add_executable(cnfkit-tests
  ClauseTests.cpp
  DimacsParserTests.cpp
  DratParserTests.cpp
  LiteralTests.cpp
  TestUtils.cpp
  TestUtils.h
  TernaryTests.cpp
)

target_link_libraries(cnfkit-tests PRIVATE cnfkit::cnfkit gtest gmock gmock_main)

if (CNFKIT_GNULIKE_COMPILER)
  target_compile_options(cnfkit-tests PRIVATE -Wall -Wextra -pedantic)
endif()

