// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

namespace version {

    /// @brief How should versions be compared
    enum class Schema {
        SIMPLE,
        CUSTOM,
        RPM,
        DPKG,
    };

}