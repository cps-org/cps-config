// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#include <filesystem>

namespace cps::platform {

    namespace fs = std::filesystem;

    /// @brief  Get the platform specific location that libraries are installed in
    /// @return A path segment for libraries to be installed in, relative to a prefix
    fs::path libdir();

    /// @brief  Get the platform specific location to install architecture agnostic data to
    /// @return A path segment for data to be installed in, relative to a prefix
    fs::path datadir();

} // namespace cps::platform
