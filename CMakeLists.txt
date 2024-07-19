cmake_minimum_required(VERSION 3.22)
file(READ CPS_VERSION VERSION)
string(STRIP ${VERSION} VERSION_STRIPPED)
project(
    cps-config
    LANGUAGES CXX
    VERSION ${VERSION_STRIPPED}
)

add_subdirectory(src)

if (BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
endif ()
