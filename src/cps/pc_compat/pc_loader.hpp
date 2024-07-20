// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#pragma once

#include "cps/pc_compat/pc.parser.hpp"
#include <unordered_map>

class PcLoader {
  public:
    PcLoader();

    // Properties set by pc files
    // For example, Name: libfoo
    std::unordered_map<std::string, std::string> properties;

    // Variables defined by pc files
    // For example, libdir=${PREFIX}/lib
    std::unordered_map<std::string, std::string> variables;

    void load(std::istream & istream);

    void scan_begin(std::istream & istream) const;
};

// Marking maybe_unused because scanner does not user this parameter
#define YY_DECL yy::parser::symbol_type yylex([[maybe_unused]] PcLoader & loader)
YY_DECL;
