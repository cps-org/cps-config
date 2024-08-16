// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

// This header is created to avoid writing non-trivial C++ code in pc.y.
// This header contains types and declarations that are needed by both loader and parser.

#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace cps::pc_compat {
    class PcLoader;

    // The following types needs to be declared in the parser because
    // PackageRequirement is the type of a non-terminal. bison needs
    // to do a sizeof on this type and cannot do so with a forward
    // declaration.
    enum class VersionOperation { lt, le, ne, eq, gt, ge };
    struct PackageRequirement {
        std::string package;
        std::optional<VersionOperation> operation;
        std::optional<std::string> version;

        // For parser debug output
        friend std::ostream & operator<<(std::ostream & ost, const PackageRequirement & package_requirement);
    };

    // For parser debug output
    std::ostream & operator<<(std::ostream & ost, const std::optional<VersionOperation> & version_operation);
    std::ostream & operator<<(std::ostream & ost, const std::vector<PackageRequirement> & package_requirements);
    std::ostream & operator<<(std::ostream & ost,
                              const std::variant<std::string, std::vector<PackageRequirement>> & property_value);
} // namespace cps::pc_compat
