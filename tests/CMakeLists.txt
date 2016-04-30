include_directories("${PROJECT_SOURCE_DIR}/PEGTL/unit_tests")

add_executable(test_esexpr test_esexpr.cc)

add_test(
  NAME test_esexpr
  COMMAND $<TARGET_FILE:test_esexpr>
  )
