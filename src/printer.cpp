// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "printer.hpp"
#include "error.h"
#include "loader.hpp"
#include <fmt/format.h>
// #include <json/json.h>
#include <tl/expected.hpp>

namespace printer {

    void pkgconf(const loader::Package & p, const Config & conf) {
        // XXX: assumes default_components
        // XXX: assumes everything is valid? Maybe that's fine?
        std::vector<std::string> args{};

        for (auto && c : p.default_components.value()) {

            const loader::Component & comp = p.components.find(c)->second;

            if (conf.cflags) {
                if (auto && c =
                        comp.compile_flags.find(loader::KnownLanguages::C);
                    c != comp.includes.end() && !c->second.empty()) {
                    // XXX: assumes compile flags
                    // XXX: assumes C
                    args.reserve(args.size() + c->second.size());
                    args.insert(args.end(), c->second.begin(), c->second.end());
                }
            }

            if (conf.includes) {
                if (auto && c = comp.includes.find(loader::KnownLanguages::C);
                    c != comp.includes.end() && !c->second.empty()) {
                    args.reserve(args.size() + c->second.size());
                    std::transform(c->second.begin(), c->second.end(),
                                   std::back_inserter(args),
                                   [](std::string_view s) {
                                       return fmt::format("-I{}", s);
                                   });
                }
            }

            if (conf.defines) {
                if (auto && c = comp.defines.find(loader::KnownLanguages::C);
                    c != comp.defines.end() && !c->second.empty()) {
                    auto && transformer = [](auto && d) {
                        if (d.is_define()) {
                            return fmt::format("-D{}", d.get_name());
                        } else if (d.is_undefine()) {
                            return fmt::format("-U{}", d.get_name());
                        } else {
                            return fmt::format("-D{}={}", d.get_name(), d.get_value());
                        }
                    };
                    args.reserve(args.size() + c->second.size());
                    std::transform(c->second.begin(), c->second.end(),
                                   std::back_inserter(args), transformer);
                }
            }
        }
        fmt::print("{}\n", fmt::join(args, " "));
    }

} // namespace printer
