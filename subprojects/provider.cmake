cmake_minimum_required(VERSION 3.24)

include(FetchContent)
FetchContent_Declare(
  GTest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
)
FetchContent_Declare(
  tl-expected
  URL https://github.com/TartanLlama/expected/archive/v1.1.0.tar.gz
  URL_HASH SHA256=1db357f46dd2b24447156aaf970c4c40a793ef12a8a9c2ad9e096d9801368df6
)
FetchContent_Declare(
  jsoncpp
  URL https://github.com/open-source-parsers/jsoncpp/archive/1.9.5.tar.gz
  URL_HASH SHA256=f409856e5920c18d0c2fb85276e24ee607d2a09b5e7d5f0a371368903c275da2
)
FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/10.1.1.tar.gz
  URL_HASH SHA256=78b8c0a72b1c35e4443a7e308df52498252d1cefc2b08c9a97bc9ee6cfe61f8b
)
FetchContent_Declare(
  CLI11
  URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.1.2.tar.gz
  URL_HASH SHA256=26291377e892ba0e5b4972cdfd4a2ab3bf53af8dac1f4ea8fe0d1376b625c8cb
)

macro(provide_dependency method dep_name)
  if ("${dep_name}" MATCHES "^(GTest|tl-expected|jsoncpp|fmt|CLI11)$")
    FetchContent_MakeAvailable(${dep_name})
    set(${dep_name}_FOUND TRUE)
  endif()
endmacro()

cmake_language(
  SET_DEPENDENCY_PROVIDER provide_dependency
  SUPPORTED_METHODS
    FIND_PACKAGE
)
