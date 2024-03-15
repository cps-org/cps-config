// SPDX-License-Identifier: MIT
// Copyright © 2023 Dylan Baker

#pragma once

#include "cps/env.hpp"
#include "cps/loader.hpp"

#include <tl/expected.hpp>

#include <string>
#include <vector>

namespace cps::search {

    class Result {
      public:
        Result();

        std::string version;
        loader::LangValues includes;
        loader::LangValues compile_flags;
        loader::Defines defines;
        std::vector<std::string> link_libraries;
        std::vector<std::string> link_location;
    };

    // TODO: restrictions like versions
    // TODO: caching loading packages?
    // TODO: multiple versions of packages?
    tl::expected<Result, std::string> find_package(std::string_view name, Env env);

    /// prefix_variable optional override for prefix variable
    tl::expected<Result, std::string> find_package(std::string_view name, const std::vector<std::string> & components,
                                                   bool default_components, Env env,
                                                   std::optional<std::string> prefix_variable);

} // namespace cps::search
