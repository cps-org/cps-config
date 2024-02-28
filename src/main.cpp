// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "cps-config-config.hpp"
#include "printer.hpp"
#include "search.hpp"
#include <fmt/format.h>

int main(int argc, char * argv[]) {
    if (argc < 2) {
        fmt::println(stderr, "Error: Got {} arguments, expected at least 2", argc);
        return 1;
    }

    printer::Config conf{};
    std::vector<std::string> components;
    std::string format{"pkgconf"};

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
            components.push_back(argv[++i]);
        } else if (arg.find("--component=") != std::string::npos) {
            components.emplace_back(arg.substr(0, 13));
        } else if (arg == "--version") {
            fmt::println(VERSION);
            return 0;
        } else if (arg == "--format") {
            format = argv[++i];
        } else if (arg.substr(0, 10) == "--format=") {
            format = arg.substr(10, arg.length());
        } else if (arg.substr(0, 9) != "--format=") {
            fmt::println(stderr, "Unknown command like argument {}", arg);
            return 1;
        }
    }

    auto && p = search::find_package(argv[1], components, components.empty());
    if (!p) {
        fmt::println(p.error());
        return 1;
    }
    auto && result = p.value();

    if (format == "pkgconf") {
        return printer::pkgconf(result, conf);
        return 0;
    }

    fmt::println(stderr, "Unknown mode {}", format);
    return 1;
}