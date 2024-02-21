// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#pragma once

#include "cps-config-config.hpp"
#include <cassert>

#if HAVE_UNREACHABLE
#define unreachable(msg)                                                       \
    do {                                                                       \
        assert(!"" && msg);                                                    \
        __builtin_unreachable();                                               \
    } while (0)
#else
#define unreachable(str) assert(!"" str);
#endif
