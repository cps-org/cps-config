// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "printer.hpp"
#include "error.h"
#include "loader.hpp"
#include <fmt/format.h>
// #include <json/json.h>
#include <tl/expected.hpp>

namespace printer {

    int pkgconf(const loader::Package & p, const Config & conf) {
        // XXX: assumes everything is valid? Maybe that's fine?
        std::vector<std::string> args{};

        const std::vector<std::string> & components =
            conf.components.empty() ? p.default_components.value()
                                    : conf.components;

        for (auto && c : components) {

            auto && trial = p.components.find(c);
            if (trial == p.components.end()) {
                fmt::println(stderr, "Component {} not found in package {}", c,
                             p.name);
                return 1;
            }
            const loader::Component comp = trial->second;

            if (conf.cflags) {
                if (auto && f =
                        comp.compile_flags.find(loader::KnownLanguages::C);
                    f != comp.includes.end() && !f->second.empty()) {
                    // XXX: assumes compile flags
                    // XXX: assumes C
                    args.reserve(args.size() + f->second.size());
                    args.insert(args.end(), f->second.begin(), f->second.end());
                }
            }

            if (conf.includes) {
                if (auto && f = comp.includes.find(loader::KnownLanguages::C);
                    f != comp.includes.end() && !f->second.empty()) {
                    args.reserve(args.size() + f->second.size());
                    std::transform(f->second.begin(), f->second.end(),
                                   std::back_inserter(args),
                                   [](std::string_view s) {
                                       return fmt::format("-I{}", s);
                                   });
                }
            }

            if (conf.defines) {
                if (auto && f = comp.defines.find(loader::KnownLanguages::C);
                    f != comp.defines.end() && !f->second.empty()) {
                    auto && transformer = [](auto && d) {
                        if (d.is_define()) {
                            return fmt::format("-D{}", d.get_name());
                        } else if (d.is_undefine()) {
                            return fmt::format("-U{}", d.get_name());
                        } else {
                            return fmt::format("-D{}={}", d.get_name(),
                                               d.get_value());
                        }
                    };
                    args.reserve(args.size() + f->second.size());
                    std::transform(f->second.begin(), f->second.end(),
                                   std::back_inserter(args), transformer);
                }
            }

            if (conf.libs_link) {
                if (auto && f = comp.link_libraries; !f.empty()) {
                    args.reserve(args.size() + f.size());
                    std::transform(f.begin(), f.end(), std::back_inserter(args),
                                   [](std::string_view s) {
                                       return fmt::format("-l{}", s);
                                   });
                }
            }
        }
        fmt::print("{}\n", fmt::join(args, " "));
        return 0;
    }

} // namespace printer
