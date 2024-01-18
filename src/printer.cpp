// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "printer.hpp"
#include "error.h"
#include "loader.hpp"
#include <fmt/format.h>
// #include <json/json.h>
#include <tl/expected.hpp>

namespace printer {

    namespace {

        tl::expected<const loader::Component *, std::string>
        get_component(const std::string & name,
                      const std::unordered_map<std::string, loader::Component> &
                          components) {
            if (auto && comp = components.find(name);
                comp != components.end()) {
                return &comp->second;
            }

            return tl::unexpected(
                fmt::format("Component {} not found in component list", name));
        }

        tl::expected<std::vector<const loader::Component *>, std::string>
        get_components(const loader::Package & p) {
            std::vector<const loader::Component *> comps{};

            for (auto && name : p.default_components.value()) {
                comps.push_back(TRY(get_component(name, p.components)));
            }

            return comps;
        }

    } // namespace

    void pkgconf(const loader::Package & p, const Config & conf) {
        // XXX: assumes default_components
        // XXX: assumes there is only one default component
        // XXX: assumes everything is valid? Maybe that's fine?
        const std::string & c = p.default_components.value().front();
        const loader::Component & comp = p.components.find(c)->second;

        if (conf.cflags) {
            // XXX: assumes compile flags
            // XXX: assumes C
            fmt::println("{}", fmt::join(comp.compile_flags.value().at(loader::KnownLanguages::C), " "));
        }
    }

} // namespace printer
