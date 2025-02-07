// SPDX-License-Identifier: MIT
// Copyright © 2024-2025 Dylan Baker
// Copyright © 2024 Bret Brown

#pragma once

#include <string>
#include <vector>

namespace cps::utils {

    void assert_fn(bool expr, std::string_view msg);

    std::vector<std::string> split(std::string_view input, std::string_view delim = ":");

} // namespace cps::utils
