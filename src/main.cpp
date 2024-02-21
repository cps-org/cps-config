// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include <fmt/format.h>
#include "loader.hpp"
#include "printer.hpp"
#include "cps-config-config.hpp"

int main(int argc, char * argv[]) {
    if (argc < 3) {
        fmt::println(stderr, "Error: Got wrong number of arguments, expected at least 3");
        return 1;
    }

    const std::string_view mode = argv[1];
    const std::string_view cps = argv[2];
    if (cps == "--version") {
        fmt::println(VERSION);
        return 0;
    }

    std::filesystem::path p{cps};
    const loader::Package package =
        loader::load(p)
            .map_error([](const std::string & v) { throw std::runtime_error(v); })
            .value();

    printer::Config conf{};

    if (mode == "pkgconf") {
        if (argc > 3) {
            for (int i = 3; i < argc; ++i) {
                const std::string_view arg = argv[i];
                if (arg == "--cflags") {
                } else if (arg == "--cflags-only-other") {
                    conf.includes = false;
                } else if (arg == "--cflags-only-I") {
                    conf.cflags = false;
                    conf.defines = false;
                } else if (arg == "--component") {
                    // TODO: error handling
                    conf.components.push_back(argv[++i]);
                } else if (arg.find("--component=") != std::string::npos) {
                    conf.components.emplace_back(arg.substr(0, 13));
                } else if (arg == "--version") {
                    fmt::println(VERSION);
                    return 0;
                } else {
                    fmt::println(stderr, "Unknown command like argument {}", arg);
                    return 1;
                }
            }
        }
        return printer::pkgconf(package, conf);
    }

    fmt::println(stderr, "Unknown mode {}", mode);
    return 1;
}