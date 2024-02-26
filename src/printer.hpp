// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include "search.hpp"

namespace printer {

    struct Config {
        bool defines = false;
        bool includes = false;
        bool cflags = false;
        bool libs_link = false;
        bool libs_search = false;
        bool libs_other = false;
        bool mod_version = false;
    };

    int pkgconf(const search::Result & dag, const Config & conf);

}