// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#pragma once

#include "cps/search.hpp"

namespace cps::printer {

    struct Config {
        bool defines = false;
        bool includes = false;
        bool cflags = false;
        bool libs_link = false;
        bool libs_search = false;
        bool libs_other = false;
        bool mod_version = false;
        bool print_errors = false;
    };

    int pkgconf(const search::Result & dag, const Config & conf);

} // namespace cps::printer
