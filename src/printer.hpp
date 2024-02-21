// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include "loader.hpp"

namespace printer {

    struct Config {
        bool defines = false;
        bool includes = false;
        bool cflags = false;
        bool libs_link = false;
        bool libs_search = false;
        bool libs_other = false;
        std::vector<std::string> components{}; // If empty Default-Components will be used
    };

    int pkgconf(const loader::Package & p, const Config & conf);

}