// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "cps-config-config.hpp"
#include "loader.hpp"
#include "printer.hpp"
#include <fmt/format.h>

int main(int argc, char * argv[]) {
    if (argc < 2) {
        fmt::println(
            stderr,
            "Error: Got wrong number of arguments, expected at least 2");
        return 1;
    }

    const std::string_view cps = argv[1];

    std::filesystem::path p{cps};
    const loader::Package package = loader::load(p)
                                        .map_error([](const std::string & v) {
                                            throw std::runtime_error(v);
                                        })
                                        .value();

    printer::Config conf{};

    const std::string_view mode = [&]() -> std::string_view {
        for (int i = 2; i < argc; ++i) {
            const std::string_view arg = argv[i];
            if (arg == "--format") {
                return argv[i + 1];
            } else if (arg.substr(0, 10) == "--format=") {
                return arg.substr(10, arg.length());
            }
        }
        return "pkgconf";
    }();

    if (mode == "pkgconf") {
        if (argc > 2) {
            for (int i = 2; i < argc; ++i) {
                const std::string_view arg = argv[i];
                if (arg == "--cflags") {
                    conf.cflags = true;
                    conf.defines = true;
                    conf.includes = true;
                } else if (arg == "--cflags-only-other") {
                    conf.cflags = true;
                    conf.defines = true;
                } else if (arg == "--cflags-only-I") {
                    conf.includes = true;
                } else if (arg == "--libs") {
                    conf.libs_link = true;
                    conf.libs_search = true;
                    conf.libs_other = true;
                } else if (arg == "--libs-only-l") {
                    conf.libs_link = true;
                } else if (arg == "--libs-only-L") {
                    conf.libs_search = true;
                } else if (arg == "--libs-only-other") {
                    conf.libs_other = true;
                } else if (arg == "--modversion") {
                    conf.mod_version = true;
                } else if (arg == "--component") {
                    // TODO: error handling
                    conf.components.push_back(argv[++i]);
                } else if (arg.find("--component=") != std::string::npos) {
                    conf.components.emplace_back(arg.substr(0, 13));
                } else if (arg == "--version") {
                    fmt::println(VERSION);
                    return 0;
                } else if (arg == "--format") {
                    // Roll forward to consume argument to --format
                    i++;
                } else if (arg.substr(0, 9) != "--format=") {
                    fmt::println(stderr, "Unknown command like argument {}",
                                 arg);
                    return 1;
                }
            }
        }
        return printer::pkgconf(package, conf);
    }

    fmt::println(stderr, "Unknown mode {}", mode);
    return 1;
}