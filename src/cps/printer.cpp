// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/printer.hpp"

#include <algorithm>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tl/expected.hpp>

namespace cps::printer {

    namespace fs = std::filesystem;

    int pkgconf(const search::Result & r, const Config & conf) {
        std::vector<std::string> args{};

        if (conf.mod_version) {
            fmt::print("{}\n", r.version);
            return 0;
        }

        if (conf.cflags) {
            if (auto && f = r.compile_flags.find(loader::KnownLanguages::c);
                f != r.compile_flags.end() && !f->second.empty()) {
                // XXX: assumes compile flags
                // XXX: assumes C
                args.reserve(args.size() + f->second.size());
                args.insert(args.end(), f->second.begin(), f->second.end());
            }
        }

        if (conf.includes) {
            if (auto && f = r.includes.find(loader::KnownLanguages::c); f != r.includes.end() && !f->second.empty()) {
                std::transform(f->second.begin(), f->second.end(), std::back_inserter(args),
                               [](const fs::path & p) { return fmt::format("-I{}", p.generic_string()); });
            }
        }

        if (conf.defines) {
            if (auto && f = r.definitions.find(loader::KnownLanguages::c);
                f != r.definitions.end() && !f->second.empty()) {
                auto && transformer = [](auto && d) {
                    if (auto && v = d.get_value()) {
                        return fmt::format("-D{}={}", d.get_name(), v.value());
                    } else {
                        return fmt::format("-D{}", d.get_name());
                    }
                };
                args.reserve(args.size() + f->second.size());
                std::transform(f->second.begin(), f->second.end(), std::back_inserter(args), transformer);
            }
        }

        if (conf.libs_search) {
            for (auto && f : r.link_flags) {
                if (f.substr(0, 2) == "-L") {
                    args.emplace_back(f);
                }
            }
        }

        if (conf.libs_other) {
            for (auto && f : r.link_flags) {
                if (auto && x = f.substr(0, 2); x != "-l" && x != "-L") {
                    args.emplace_back(f);
                }
            }
        }

        if (conf.libs_link) {
            if (auto && f = r.link_location; !f.empty()) {
                args.reserve(args.size() + f.size());
                std::transform(f.begin(), f.end(), std::back_inserter(args),
                               [](const fs::path & p) { return fmt::format("-l{}", p.generic_string()); });
            }
            if (auto && f = r.link_libraries; !f.empty()) {
                args.reserve(args.size() + f.size());
                std::transform(f.begin(), f.end(), std::back_inserter(args),
                               [](std::string_view s) { return fmt::format("-l{}", s); });
            }
            for (auto && f : r.link_flags) {
                if (f.substr(0, 2) == "-l") {
                    args.emplace_back(f);
                }
            }
        }

        fmt::print("{}\n", fmt::join(args, " "));
        return 0;
    }

} // namespace cps::printer
