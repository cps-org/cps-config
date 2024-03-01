// Copyright © 2024 Dylan Baker
// SPDX-License-Identifier: MIT

#include "cps/platform.hpp"
#include "cps/config.hpp"

namespace fs = std::filesystem;

namespace cps::platform {

    fs::path libdir() {
        // TODO: libdir needs to be configurable based on the personality,
        //       and different name schemes.
        //       This is complicated by the fact that different distros have
        //       different schemes.
        return CPS_LIBDIR;
    }

} // namespace cps::platform
