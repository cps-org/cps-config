// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#include "cps/platform.hpp"
#include "cps/config.hpp"

namespace cps::platform {

    fs::path libdir() { return CPS_CONFIG_LIBDIR; }

    fs::path datadir() { return CPS_CONFIG_DATADIR; }

} // namespace cps::platform
