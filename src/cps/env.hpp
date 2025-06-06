// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cps {

    namespace fs = std::filesystem;

    struct Env {
        std::optional<std::vector<fs::path>> cps_path = std::nullopt;
        std::optional<std::vector<fs::path>> cps_prefix_path = std::nullopt;
        std::optional<std::vector<fs::path>> pc_path = std::nullopt;
        bool debug_spew = false;
    };

    Env get_env();

} // namespace cps
