// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#pragma once

#include "cps-config-config.hpp"
#include "fmt/core.h"
#include <cassert>
#include <string>
#include <vector>

static inline void assert_fn(bool expr, std::string_view msg) {
#ifndef NDEBUG
    if (!expr) {
        fmt::println(stderr, msg);
        abort();
    }
#endif
}

#if HAVE_UNREACHABLE
#define unreachable(msg)                                                       \
    do {                                                                       \
        assert_fn(false, msg);                                                 \
        __builtin_unreachable();                                               \
    } while (0)
#else
#define unreachable(str) assert_fn(false, msg);
#endif

namespace utils {

    std::vector<std::string> split(std::string_view input,
                                   std::string_view delim = ":");

} // namespace utils