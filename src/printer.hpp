// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include "loader.hpp"

namespace printer {

    struct Config {
        bool defines = false;
        bool includes = false;
        bool cflags = false;
    };

    void pkgconf(const loader::Package & p, const Config & conf);

}