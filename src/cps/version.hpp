// Copyright © 2023 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#pragma once

#include <tl/expected.hpp>

#include <string>

namespace cps::version {

    /// @brief How should versions be compared
    enum class Schema {
        simple,
        custom,
        rpm,
        dpkg,
    };

    /// @brief The operator to compare with
    enum class Operator {
        le,
        lt,
        eq,
        ne,
        gt,
        ge,
    };

    /// @brief compare two version strings using the given operator and schema
    tl::expected<bool, std::string> compare(std::string_view left, Operator op, std::string_view right, Schema schema);
} // namespace cps::version