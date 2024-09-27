// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown

#pragma once

#include "cps/config.hpp"

#include <fmt/core.h>

#include <string>
#include <vector>

namespace cps::utils {

    inline void assert_fn(bool expr, std::string_view msg) {
        if (!expr) {
            fmt::print(stderr, "{}\n", msg);
            fflush(stderr);
            abort();
        }
    }

    std::vector<std::string> split(std::string_view input, std::string_view delim = ":");

} // namespace cps::utils
