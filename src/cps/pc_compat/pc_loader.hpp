// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#pragma once

#include "cps/pc_compat/pc.parser.hpp"

#include "cps/loader.hpp"

#include <tl/expected.hpp>
#include <unordered_map>

namespace cps::pc_compat {

    class PcLoader {
      public:
        PcLoader();

        // Properties set by pc files
        // For example, Name: libfoo
        std::unordered_map<std::string, std::string> properties;

        // Variables defined by pc files
        // For example, libdir=${PREFIX}/lib
        std::unordered_map<std::string, std::string> variables;

        tl::expected<loader::Package, std::string> load(std::istream & istream, std::string const & filename);

        void scan_begin(std::istream & istream) const;

      private:
        tl::expected<std::string, std::string> get_property(const std::string & property_name) const;
    };

    // Free function to keep a consistent interface with `cps::loader::load`
    tl::expected<loader::Package, std::string> load(std::istream & istream, std::string const & filename);

} // namespace cps::pc_compat

// Marking maybe_unused because scanner does not user this parameter
#define YY_DECL yy::parser::symbol_type yylex([[maybe_unused]] cps::pc_compat::PcLoader & loader)
YY_DECL;
