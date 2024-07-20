// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown

#include "cps/utils.hpp"

#include <algorithm>

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

    std::string_view trim(std::string_view input) {
        const auto is_not_space = [](unsigned char c) { return !std::isspace(c); };
        const size_t prefix_length =
            std::distance(input.cbegin(), std::find_if(input.cbegin(), input.cend(), is_not_space));
        input.remove_prefix(prefix_length);
        const size_t suffix_length =
            std::distance(input.crbegin(),
                          std::find_if(input.crbegin(), input.crend(), is_not_space);
        input.remove_suffix(suffix_length);
        return input;
    }

    void assert_fn(bool expr, std::string_view msg) {
        if (!expr) {
            fmt::print(stderr, "{}\n", msg);
            fflush(stderr);
            abort();
        }
    }

} // namespace cps::utils
