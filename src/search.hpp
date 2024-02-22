// SPDX-License-Identifier: MIT
// Copyright © 2023 Dylan Baker

#pragma once

#include <tl/expected.hpp>
#include <filesystem>

namespace search {

    // TODO: restrictions like versions
    // TODO: caching loading packages?
    // TODO: multiple versions of packages?
    tl::expected<std::filesystem::path, std::string> find_package(std::string_view name);

}