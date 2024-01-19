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

    const std::string_view mode = argv[1];
    const std::string_view cps = argv[2];

    std::filesystem::path p{cps};
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

    if (mode == "pkgconf") {
        printer::pkgconf(package, conf);
        return 0;
    } else {
        fmt::println(stderr, "Unknown mode {}", mode);
    }

    return 1;
}