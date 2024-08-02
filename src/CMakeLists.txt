# PC file parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
bison_target(PcParser cps/pc_compat/pc.y ${CMAKE_CURRENT_BINARY_DIR}/cps/pc_compat/pc.parser.cpp)
flex_target(PcScanner cps/pc_compat/pc.l ${CMAKE_CURRENT_BINARY_DIR}/cps/pc_compat/pc.lexer.cpp)
add_flex_bison_dependency(PcScanner PcParser)

# cps library
add_library(
    cps
    cps/env.cpp
    cps/loader.cpp
    cps/platform.cpp
    cps/printer.cpp
    cps/search.cpp
    cps/utils.cpp
    cps/version.cpp
    ${BISON_PcParser_OUTPUTS}
    ${FLEX_PcScanner_OUTPUTS}
    cps/pc_compat/pc_loader.cpp
)

# Configure config.hpp
configure_file(cps/config.hpp.in cps/config.hpp)

target_include_directories(cps PUBLIC .)
target_include_directories(cps PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(cps PRIVATE ${FLEX_LIBRARIES})

# Dependencies
find_package(tl-expected 1.0 REQUIRED)
target_link_libraries(cps PRIVATE tl::expected)

find_package(fmt 8 REQUIRED)
target_link_libraries(cps PRIVATE fmt::fmt)

find_package(jsoncpp 1.9 REQUIRED)
# Provide jsoncpp namespaced target when it's not available
if (NOT TARGET JsonCpp::JsonCpp)
    if (BUILD_SHARED_LIBS)
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_lib)
    else ()
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_static)
    endif ()
endif ()
target_link_libraries(cps PRIVATE JsonCpp::JsonCpp)

# cps-config
add_executable(cps-config cps-config/main.cpp)
target_link_libraries(cps-config PRIVATE cps fmt::fmt)

find_package(CLI11 2.1 REQUIRED)
target_link_libraries(cps-config PRIVATE CLI11::CLI11)
