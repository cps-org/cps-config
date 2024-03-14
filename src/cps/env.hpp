// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <string>

namespace cps {

    struct Env {
        std::optional<std::string> cps_path = std::nullopt;
        bool debug_spew = false;
    };

    Env get_env();

} // namespace cps
