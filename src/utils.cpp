// SPDX-License-Identifier: MIT
// Copyright © 2024 Dylan Baker

#include "utils.hpp"

namespace utils {

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

} // namespace utils