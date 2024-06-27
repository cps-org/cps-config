find_package(GTest 1.10 REQUIRED)

# Unit tests
add_executable(cps-tests
  loader.cpp
  utils.cpp
  version.cpp
)
target_link_libraries(cps-tests PRIVATE cps)
target_link_libraries(cps-tests PRIVATE
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cps-tests)

# Integration tests
find_package(
  Python 3.11
  COMPONENTS Interpreter
  REQUIRED
)
add_test(
  NAME pkg-config-compatibility-tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND
    ${CMAKE_COMMAND} -E env CPS_PATH=${CMAKE_CURRENT_SOURCE_DIR}/cases
    ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runner.py
    $<TARGET_FILE:cps-config> ${CMAKE_CURRENT_SOURCE_DIR}/cases.toml
)
