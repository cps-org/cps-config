// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#include "cps/platform.hpp"

namespace cps::platform {

    fs::path libdir() { return "lib"; }

    fs::path datadir() { return "share"; }

} // namespace cps::platform
