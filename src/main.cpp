// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include <fmt/format.h>
#include "loader.hpp"

int main(int argc, char * argv[]) {
    if (argc != 2) {
        fmt::println(stderr, "Error: Got wrong number of arguments, expected 2");
        return 1;
    }

    std::filesystem::path p{argv[1]};
    const loader::Package package =
        loader::load(p)
            .map_error([](const std::string & v) { throw std::runtime_error(v); })
            .value();

    fmt::println("name is: {}", package.name);
    fmt::println("uses cps-version: {}", package.cps_version);
    for (auto && [k, v] : package.components) {
        fmt::println("has component: {}", k);
    }

    return 0;
}