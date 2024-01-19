// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include <fmt/format.h>
#include "loader.hpp"
#include "printer.hpp"

int main(int argc, char * argv[]) {
    if (argc < 3) {
        fmt::println(stderr, "Error: Got wrong number of arguments, expected at least 3");
        return 1;
    }

    std::filesystem::path p{argv[1]};
    const loader::Package package =
        loader::load(p)
            .map_error([](const std::string & v) { throw std::runtime_error(v); })
            .value();

    printer::Config conf{};
    if (argc > 3) {
        for (int i = 3; i < argc; ++i) {
            const std::string_view arg = argv[i];
            if (arg == "--cflags") {
                conf.cflags = true;
            }
        }
    }

    const std::string_view mode = argv[2];
    if (mode == "pkgconf") {
        printer::pkgconf(package, conf);
    } else {
        fmt::println(stderr, "Unknown mode {}", argv[2]);
        return 1;
    }

    return 0;
}