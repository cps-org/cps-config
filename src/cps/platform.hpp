// Copyright © 2024 Dylan Baker
// SPDX-License-Identifier: MIT

/**
 * Helpers to abstract the target platform.
 */

#pragma once

#include <filesystem>

namespace cps::platform {

    std::filesystem::path libdir();

} // namespace cps::platform
