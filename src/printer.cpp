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
        // XXX: assumes there is only one default component
        // XXX: assumes everything is valid? Maybe that's fine?
        const std::string & c = p.default_components.value().front();
        const loader::Component & comp = p.components.find(c)->second;

        bool need_space = false;
        if (conf.cflags) {
            if (auto && c = comp.compile_flags.find(loader::KnownLanguages::C);
                c != comp.includes.end() && !c->second.empty()) {
                // XXX: assumes compile flags
                // XXX: assumes C
                fmt::print("{}", fmt::join(c->second, " "));
                need_space = true;
            }
        }
        if (conf.includes) {
            if (auto && c = comp.includes.find(loader::KnownLanguages::C);
                c != comp.includes.end() && !c->second.empty()) {
                if (need_space) {
                    fmt::print(" ");
                }
                fmt::print("-I{}", fmt::join(c->second, " -I"));
                need_space = true;
            }
        }
        fmt::print("\n");
    }

} // namespace printer
