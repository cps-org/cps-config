// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cps {

    struct Env {
        std::optional<std::string> cps_path = std::nullopt;
        std::optional<std::vector<std::string>> cps_prefix_path = std::nullopt;
        std::optional<std::string> pc_path = std::nullopt;
        bool debug_spew = false;
    };

    Env get_env();

} // namespace cps
