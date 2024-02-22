// SPDX-License-Identifier: MIT
// Copyright © 2023 Dylan Baker

#pragma once

#include <tl/expected.hpp>
#include "loader.hpp"

namespace search {

    // TODO: restrictions like versions
    // TODO: caching loading packages?
    // TODO: multiple versions of packages?
    tl::expected<loader::Package, std::string> find_package(std::string_view name);

}