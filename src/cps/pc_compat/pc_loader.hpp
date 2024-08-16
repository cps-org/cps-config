// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#pragma once

#include "cps/loader.hpp"
#include "cps/pc_compat/pc.parser.hpp"
#include "cps/pc_compat/pc_base.hpp"

#include <filesystem>
#include <unordered_map>
#include <variant>

#include <tl/expected.hpp>

namespace cps::pc_compat {

    using PackageRequirements = std::vector<PackageRequirement>;
    using PcPropertyValue = std::variant<std::string, PackageRequirements>;

    class PcLoader {
      public:
        PcLoader();

        // Properties set by pc files
        // For example, Name: libfoo
        std::unordered_map<std::string, PcPropertyValue> properties;

        // Variables defined by pc files
        // For example, libdir=${PREFIX}/lib
        std::unordered_map<std::string, std::string> variables;

        tl::expected<loader::Package, std::string> load(std::istream & istream, std::filesystem::path const & filename);

        void scan_begin(std::istream & istream) const;

      private:
        tl::expected<PcPropertyValue, std::string> get_property(const std::string & property_name) const;

        static tl::expected<std::string, std::string> get_string(const PcPropertyValue & property_value);
        static tl::expected<PackageRequirements, std::string>
        get_package_requirements(const PcPropertyValue & property_value);
    };

    // Free function to keep a consistent interface with `cps::loader::load`
    tl::expected<loader::Package, std::string> load(std::istream & istream, std::filesystem::path const & filename);

} // namespace cps::pc_compat

// Marking maybe_unused because scanner does not user this parameter
#define YY_DECL yy::parser::symbol_type yylex([[maybe_unused]] cps::pc_compat::PcLoader & loader)
YY_DECL;
