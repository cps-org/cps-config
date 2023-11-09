// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include <iostream>

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cerr << "Got wrong number of arguments, expected 2\n";
    }

    std::filesystem::path p{argv[1]};
    const auto package = cps_config::loader::load(p);

    std::cout << "name is: " << package.name << "\n";
    std::cout << "uses cps-version: " << package.cps_version << "\n";

    return 0;
}