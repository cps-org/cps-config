// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/config.hpp"
#include "cps/printer.hpp"
#include "cps/search.hpp"

#include <cxxopts.hpp>
#include <fmt/format.h>

#include <string>
#include <string_view>
#include <vector>

namespace cps_config {
    int run(int argc, char * argv[]) {
        using namespace std::string_literals;

        cps::printer::Config conf{};
        std::vector<std::string> components;
        std::string format{"pkgconf"};
        std::string package_name;

        static auto const description = R"(cps-config is a utility for querying and using installed libraries.

Examples:

   Getting include directory flags for a set of dependencies.
      $ cps-config --cflags-only-I fmt
      -I/usr/include\n

   Getting link directory flags for a set of dependencies.
      $ cps-config --libs-only-L fmt
      -L/usr/lib\n"s;

Support:

    Please report bugs to <https://github.com/cps-org/cps-config/issues>.
)"s;
        cxxopts::Options options("cps-config", description);
        // clang-format off
        options.add_options()
            ("package", "search for the specified package", cxxopts::value<std::string>())
            ("cflags", "output all pre-processor and compiler flags")
            ("cflags-only-I", "output -I flags")
            ("cflags-only-other", "output cflags not covered by the cflags-only-I option")
            ("libs", "output all linker flags")("libs-only-L", "print required LDPATH linker flags to stdout")
            ("libs-only-l", "print required LIBNAME linker flags to stdout")
            ("libs-only-other", "print required other linker flags to stdout")
            ("modversion", "print the specified module's version to stdout")
            ("component", "look for the specified component", cxxopts::value<std::vector<std::string>>())
            ("version", "print cps-config version")
            ("format", "output format", cxxopts::value<std::string>())
            ("h,help", "print usage");
        // clang-format on
        options.parse_positional({"package"});
        options.positional_help("<packages>");
        auto parsed_options = options.parse(argc, argv);

        if (parsed_options.count("help")) {
            fmt::print("{}\n", options.help());
            return 0;
        }
        if (parsed_options.count("package")) {
            package_name = parsed_options["package"].as<std::string>();
        } else {
            fmt::print(stderr, "Expected a package name to be specified\n");
            return 1;
        }

        if (parsed_options.count("cflags")) {
            conf.cflags = true;
            conf.defines = true;
            conf.includes = true;
        }
        if (parsed_options.count("cflags-only-other")) {
            conf.cflags = true;
            conf.defines = true;
        }
        if (parsed_options.count("cflags-only-I")) {
            conf.includes = true;
        }
        if (parsed_options.count("libs")) {
            conf.libs_link = true;
            conf.libs_search = true;
            conf.libs_other = true;
        }
        if (parsed_options.count("libs-only-l")) {
            conf.libs_link = true;
        }
        if (parsed_options.count("libs-only-L")) {
            conf.libs_search = true;
        }
        if (parsed_options.count("libs-only-other")) {
            conf.libs_other = true;
        }
        if (parsed_options.count("modversion")) {
            conf.mod_version = true;
        }
        if (parsed_options.count("component")) {
            components = parsed_options["component"].as<std::vector<std::string>>();
        }
        if (parsed_options.count("version")) {
            fmt::print("{}\n", CPS_VERSION);
            return 0;
        }
        if (parsed_options.count("format")) {
            format = parsed_options["format"].as<std::string>();
        }

        auto && p = cps::search::find_package(package_name, components, components.empty());
        if (!p) {
            fmt::print("{}\n", p.error());
            return 1;
        }
        auto && result = p.value();

        if (format == "pkgconf") {
            return cps::printer::pkgconf(result, conf);
            return 0;
        }

        fmt::print(stderr, "Unknown mode {}\n", format);
        return 1;
    }
} // namespace cps_config

int main(int argc, char * argv[]) { return cps_config::run(argc, argv); }
