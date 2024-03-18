// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/config.hpp"
#include "cps/env.hpp"
#include "cps/printer.hpp"
#include "cps/search.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace cps_config {
    struct ProgramOutput {
        int retval = 0;
        std::string debug_output = "";
        bool errors_to_stdout = false;

        static auto Success() { return ProgramOutput{}; }
    };

    ProgramOutput run(int argc, char * argv[]) {
        using namespace std::string_literals;

        cps::printer::Config conf{};
        std::vector<std::string> components;
        std::string format{"pkgconf"};
        std::vector<std::string> package_names;
        bool errors_to_stdout = false;
        std::optional<std::string> prefix_variable = std::nullopt;

        // read enviroment variables
        auto env = cps::get_env();

        static auto const footer = R"(Examples:

   Getting include directory flags for a set of dependencies.
      $ cps-config --cflags-only-I fmt
      -I/usr/include\n

   Getting link directory flags for a set of dependencies.
      $ cps-config --libs-only-L fmt
      -L/usr/lib\n"s;

Support:

    Please report bugs to <https://github.com/cps-org/cps-config/issues>.)"s;
        CLI::App app{"cps-config is a utility for querying and using installed libraries."};
        app.footer(footer);

        app.add_flag_callback(
            "--cflags",
            [&conf]() {
                conf.cflags = true;
                conf.defines = true;
                conf.includes = true;
            },
            "output all pre-processor and compiler flags");
        app.add_flag("--cflags-only-I", conf.includes, "output -I flags");
        app.add_flag_callback(
            "--cflags-only-other",
            [&conf]() {
                conf.cflags = true;
                conf.defines = true;
            },
            "output cflags not covered by the cflags-only-I option");
        app.add_flag_callback(
            "--libs",
            [&conf]() {
                conf.libs_link = true;
                conf.libs_search = true;
                conf.libs_other = true;
            },
            "output all linker flags");
        app.add_flag("--libs-only-L", conf.libs_search, "print required LDPATH linker flags to stdout");
        app.add_flag("--libs-only-l", conf.libs_link, "print required LIBNAME linker flags to stdout");
        app.add_flag("--libs-only-other", conf.libs_other, "print required other linker flags to stdout");
        app.add_flag("--modversion", conf.mod_version, "print the specified module's version to stdout");
        app.add_option<std::vector<std::string>>("--component"s, components, "look for the specified component(s)"s);
        app.add_flag("--format", format, "output format");
        app.add_flag("--print-errors", conf.print_errors, "enables debug messages when errors are encountered");
        app.add_flag("--errors-to-stdout", errors_to_stdout, "print errors to stdout instead of stderr");
        app.add_flag("--prefix-variable", prefix_variable,
                     "set value of @prefix@ instead of infering it from where the cps file was found");
        app.set_version_flag("--version", CPS_CONFIG_VERSION, "print cps-config version");
        app.add_option("packages", package_names, "search for the specified packages")->required();
        try {
            app.parse(argc, argv);
        } catch (const CLI ::ParseError & parse_error) {
            std::ostringstream error_out;
            int const retval = app.exit(parse_error);
            return ProgramOutput{
                .retval = retval, .debug_output = error_out.str(), .errors_to_stdout = errors_to_stdout};
        }

        auto && p =
            cps::search::find_package(package_names.front(), components, components.empty(), env, prefix_variable);
        if (!p) {
            return ProgramOutput{.retval = 1,
                                 .debug_output = conf.print_errors ? fmt::format("{}\n", p.error()) : "",
                                 .errors_to_stdout = errors_to_stdout};
        }
        auto && result = p.value();

        if (format == "pkgconf") {
            auto retval = cps::printer::pkgconf(result, conf);
            return ProgramOutput{.retval = retval};
        }

        return ProgramOutput{.retval = 1,
                             .debug_output = conf.print_errors ? fmt::format("Unknown mode {}\n", format) : "",
                             .errors_to_stdout = errors_to_stdout};
    }
} // namespace cps_config

int main(int argc, char * argv[]) {
    auto result = cps_config::run(argc, argv);
    if (!result.debug_output.empty()) {
        if (result.errors_to_stdout) {
            fmt::print(stdout, "{}", result.debug_output);
        } else {
            fmt::print(stderr, "{}", result.debug_output);
        }
    }
    return result.retval;
}
