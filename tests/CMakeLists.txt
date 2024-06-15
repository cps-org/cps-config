find_package(GTest REQUIRED)

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
add_test(CpsTests cps-tests)
