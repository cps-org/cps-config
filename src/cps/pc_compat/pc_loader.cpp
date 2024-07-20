// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#include "cps/pc_compat/pc_loader.hpp"

#include "cps/pc_compat/pc.parser.hpp"

PcLoader::PcLoader() = default;

void PcLoader::load(std::istream & istream) {
    scan_begin(istream);
    yy::parser parse(*this);
    // To debug parser, uncomment the following line
    // TODO: add a way to enable debug output without rebuilding
        // parse.set_debug_level(true);
    if (const int result = parse(); result != 0) {
        throw std::runtime_error("Failed to parse the given pkg-config file.");
    }
}
