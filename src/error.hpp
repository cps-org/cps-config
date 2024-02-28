// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include <tl/expected.hpp>

#define TRY(expr)                                                                                                      \
    ({                                                                                                                 \
        auto && t_expect = (expr);                                                                                     \
        if (not t_expect)                                                                                              \
            return tl::unexpected(std::move(t_expect.error()));                                                        \
        std::move(t_expect.value());                                                                                   \
    })
