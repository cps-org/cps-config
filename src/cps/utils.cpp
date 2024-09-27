// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown

#include "cps/utils.hpp"

#include <fmt/core.h>

namespace cps::utils {

    std::vector<std::string> split(std::string_view input, std::string_view delim) {
        size_t last = 0, next = 0;
        std::vector<std::string> out;

        while ((next = input.find(delim, last)) != std::string_view::npos) {
            out.emplace_back(input.substr(last, next - last));
            last = next + 1;
        }
        out.emplace_back(input.substr(last));

        return out;
    }

    void assert_fn(bool expr, std::string_view msg) {
        if (!expr) {
            fmt::print(stderr, "{}\n", msg);
            fflush(stderr);
            abort();
        }
    }

} // namespace cps::utils
