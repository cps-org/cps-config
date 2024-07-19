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
set(test_names "cps integration tests" "pkg-config compatibility")
set(test_cases
    "${CMAKE_CURRENT_SOURCE_DIR}/cases/cps-config.toml"
    "${CMAKE_CURRENT_SOURCE_DIR}/cases/pkg-config-compat.toml")
foreach (test_name test_case IN ZIP_LISTS test_names test_cases)
    add_test(
        NAME ${test_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND
        ${CMAKE_COMMAND} -E env CPS_PREFIX_PATH=${CMAKE_CURRENT_SOURCE_DIR}/cps-files
        ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runner.py
        $<TARGET_FILE:cps-config> ${test_case}
    )
endforeach ()
