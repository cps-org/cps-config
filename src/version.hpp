// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace version {

    /// @brief How should versions be compared
    enum class Schema {
        SIMPLE,
        CUSTOM,
        RPM,
        DPKG,
    };

    /// @brief The operator to compare with
    enum class Operator {
        LE,
        LT,
        EQ,
        NE,
        GT,
        GE,
    };

    /// @brief compare two version strings using the given operator and schema
    bool compare(std::string_view left, Operator op, std::string_view right, Schema schema);
}