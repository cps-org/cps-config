find_package(GTest 1.10 REQUIRED)

# Unit tests
add_executable(cps-tests
    loader.cpp
    utils.cpp
    version.cpp
    pc_parser.cpp
)
target_link_libraries(cps-tests PRIVATE cps)
target_link_libraries(cps-tests PRIVATE
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cps-tests PROPERTIES
    ENVIRONMENT_MODIFICATION
    "CPS_TEST_DIR=set:${CMAKE_CURRENT_SOURCE_DIR}"
)

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
        PKG_CONFIG_PATH=${CMAKE_CURRENT_SOURCE_DIR}/cps-files/lib/pkgconfig
        ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runner.py
        $<TARGET_FILE:cps-config> ${test_case} --libdir "${CMAKE_INSTALL_LIBDIR}"
    )
endforeach ()

set(infiles
    "cps-files/lib/cps/cps-path-set.cps")
set(libdir ${CMAKE_INSTALL_LIBDIR})
set(prefix "@prefix@")  # Work around for not having the inverse of @ONLY
foreach(infile ${infiles})
  configure_file("${infile}.in" "${infile}")
endforeach()

add_test(
    NAME "prefix calculation tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND
    ${CMAKE_COMMAND} -E env CPS_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/cps-files
    ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runner.py
    $<TARGET_FILE:cps-config> ${CMAKE_CURRENT_SOURCE_DIR}/cases/cps-prefix-calculation.toml --libdir "${CMAKE_INSTALL_LIBDIR}" --prefix ${PROJECT_BINARY_DIR}
)
