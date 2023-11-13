// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include <iostream>

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cerr << "Got wrong number of arguments, expected 2\n";
    }

    std::filesystem::path p{argv[1]};
    const auto package =
        loader::load(p)
            .map_error([](const std::string & v) { throw std::runtime_error(v); })
            .value();

    std::cout << "name is: " << package.name << "\n";
    std::cout << "uses cps-version: " << package.cps_version << "\n";
    for (auto && [k, v] : package.components) {
        std::cout << "has component: " << k << "\n";
    }

    return 0;
}