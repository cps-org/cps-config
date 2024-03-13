// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/printer.hpp"

#include "cps/error.hpp"

#include <fmt/format.h>
#include <tl/expected.hpp>

namespace cps::printer {

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
                               [](std::string_view s) { return fmt::format("-I{}", s); });
            }
        }

        if (conf.defines) {
            if (auto && f = r.defines.find(loader::KnownLanguages::c); f != r.defines.end() && !f->second.empty()) {
                auto && transformer = [](auto && d) {
                    if (d.is_define()) {
                        return fmt::format("-D{}", d.get_name());
                    } else if (d.is_undefine()) {
                        return fmt::format("-U{}", d.get_name());
                    } else {
                        return fmt::format("-D{}={}", d.get_name(), d.get_value());
                    }
                };
                args.reserve(args.size() + f->second.size());
                std::transform(f->second.begin(), f->second.end(), std::back_inserter(args), transformer);
            }
        }

        if (conf.libs_link) {
            if (auto && f = r.link_location; !f.empty()) {
                args.reserve(args.size() + f.size());
                std::transform(f.begin(), f.end(), std::back_inserter(args),
                               [](std::string_view s) { return fmt::format("-l{}", s); });
            }
            if (auto && f = r.link_libraries; !f.empty()) {
                args.reserve(args.size() + f.size());
                std::transform(f.begin(), f.end(), std::back_inserter(args),
                               [](std::string_view s) { return fmt::format("-l{}", s); });
            }
        }

        fmt::print("{}\n", fmt::join(args, " "));
        return 0;
    }

} // namespace printer
