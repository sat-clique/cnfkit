if (CNFKIT_ENABLE_TESTS)
  add_executable(cnfkit-tests
    clause_tests.cpp
    dimacs_parser_tests.cpp
    drat_parser_tests.cpp
    drat_writer_tests.cpp
    literal_tests.cpp
    test_utils.cpp
    test_utils.h
    ternary_tests.cpp
  )

  target_link_libraries(cnfkit-tests PRIVATE cnfkit gtest gmock gmock_main)

  if (CNFKIT_GNULIKE_COMPILER)
    target_compile_options(cnfkit-tests PRIVATE -Wall -Wextra -pedantic)
  endif()
endif()

