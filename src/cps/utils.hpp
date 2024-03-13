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
#ifndef NDEBUG
        if (!expr) {
            fmt::print(stderr, "{}\n", msg);
            abort();
        }
#endif
    }

} // namespace cps::utils

#if CPS_USE_BUILTIN_UNREACHABLE
#define CPS_UNREACHABLE(msg)                                                                                           \
    do {                                                                                                               \
        cps::utils::assert_fn(false, msg);                                                                             \
        __builtin_unreachable();                                                                                       \
    } while (0)
#else
#define CPS_UNREACHABLE(msg) cps::utils::assert_fn(false, msg);
#endif

namespace cps::utils {

    std::vector<std::string> split(std::string_view input, std::string_view delim = ":");

} // namespace cps::utils
